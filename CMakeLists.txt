cmake_minimum_required(VERSION 3.5)
project(mt_queue)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(mt_nodes src/main.cpp)
ament_target_dependencies(
  mt_nodes
  rclcpp
  std_msgs
)

# include headers
target_include_directories(mt_nodes
  PUBLIC
    include
)

install(TARGETS
  mt_nodes
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
