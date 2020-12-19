#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <queue>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/experimental/buffers/ring_buffer_implementation.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

std::string string_thread_id()
{
  auto hashed = std::hash<std::thread::id>()(std::this_thread::get_id());
  return std::to_string(hashed);
}

class ReaderNode : public rclcpp::Node
{
public:
  ReaderNode(rclcpp::experimental::buffers::RingBufferImplementation<uint8_t> *buffer)
  : Node("ReaderNode"), data_(0)
  {
    buffer_ = buffer;

    auto timer_callback =      
      [this]() -> void {        
        auto curr_thread = string_thread_id();

        // Enqueue a byte into ring buffer
        buffer_->enqueue(data_++);

        // Prep display message
        RCLCPP_INFO(
          this->get_logger(), "<<THREAD %s>> Push 1 byte into ringbuffer",
          curr_thread.c_str());
      };
    timer_ = this->create_wall_timer(100ms, timer_callback);
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  uint8_t data_;

  rclcpp::experimental::buffers::RingBufferImplementation<uint8_t> *buffer_;
};

class ParserNode : public rclcpp::Node
{
public:
  ParserNode(rclcpp::experimental::buffers::RingBufferImplementation<uint8_t> *buffer)
  : Node("ParserNode"), count_(0)
  {
    buffer_ = buffer;

    publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
    auto timer_callback =
      [this]() -> void {
        auto message = std_msgs::msg::String();

        while(buffer_->has_data())
        {
          uint8_t byte = buffer_->dequeue();
          message.data += std::to_string(byte);
        }

        // Extract current thread
        auto curr_thread = string_thread_id();

        // Prep display message
        RCLCPP_INFO(
          this->get_logger(), "<<THREAD %s>> Publishing '%s'",
          curr_thread.c_str(), message.data.c_str());
        this->publisher_->publish(message);
      };
    timer_ = this->create_wall_timer(500ms, timer_callback);
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  size_t count_;

  rclcpp::experimental::buffers::RingBufferImplementation<uint8_t> *buffer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::executors::MultiThreadedExecutor executor;

  rclcpp::experimental::buffers::RingBufferImplementation<uint8_t> buffer(1024);

  auto reader_node = std::make_shared<ReaderNode>(&buffer);
  auto parser_node = std::make_shared<ParserNode>(&buffer);

  executor.add_node(reader_node);
  executor.add_node(parser_node);

  executor.spin();

  rclcpp::shutdown();
  return 0;
}