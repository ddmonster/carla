cmake_minimum_required(VERSION 3.5.1)
project(CARLA)

include("Build/CMakeLists.txt.in")

add_subdirectory("LibCarla/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")