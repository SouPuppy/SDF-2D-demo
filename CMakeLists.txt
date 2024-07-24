cmake_minimum_required(VERSION 3.10)
project(SDFRenderer)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(SDFRenderer main.cpp)
target_link_libraries(SDFRenderer ${OpenCV_LIBS})