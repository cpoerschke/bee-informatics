cmake_minimum_required(VERSION 3.22)

project(shades-of-bee)

find_package(OpenCV REQUIRED)

add_executable(intelligentScissors-demo common.cpp intelligentScissors-demo.cpp)
add_executable(calcHist-demo common.cpp calcHist-demo.cpp)
add_executable(calcBackProject-demo common.cpp calcBackProject-demo.cpp)
add_executable(meanShift-demo common.cpp meanShift-demo.cpp)

target_link_libraries(intelligentScissors-demo PRIVATE ${OpenCV_LIBS})
target_link_libraries(calcHist-demo PRIVATE ${OpenCV_LIBS})
target_link_libraries(calcBackProject-demo PRIVATE ${OpenCV_LIBS})
target_link_libraries(meanShift-demo PRIVATE ${OpenCV_LIBS})
