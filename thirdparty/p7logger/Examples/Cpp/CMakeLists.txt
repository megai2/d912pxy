cmake_minimum_required(VERSION 2.8)
project(P7_Cpp_Example)
include_directories(./../../Headers)
include_directories(./../../Shared)

add_executable (P7_Cpp Main.cpp)
target_link_libraries (P7_Cpp P7 pthread dl rt)

