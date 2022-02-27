cmake_minimum_required(VERSION 3.22)

project(yolo-bees-demo)

find_package(OpenCV REQUIRED)

add_executable(yolo-bees-demo demo.cpp)

target_link_libraries(yolo-bees-demo PRIVATE ${OpenCV_LIBS})
