cmake_minimum_required(VERSION 3.22)

project(background-subtractor-demo)

find_package(OpenCV REQUIRED)

add_executable(background-subtractor-demo demo.cpp)

target_link_libraries(background-subtractor-demo PRIVATE ${OpenCV_LIBS})
