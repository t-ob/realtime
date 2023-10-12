cmake_minimum_required(VERSION 3.25)
project(tensor_listener_wip)

set(CMAKE_CXX_STANDARD 20)
set(Boost_DEBUG ON)
find_package(Boost REQUIRED)

add_subdirectory(src)
add_subdirectory(bin)