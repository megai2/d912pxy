cmake_minimum_required(VERSION 2.8)
project(P7_Test_Cpp)
include_directories(./../../Headers)
include_directories(./../../Shared)
include_directories(./../../Shared/Platforms/Linux_x86)

add_executable (P7_Test Main.cpp)
target_link_libraries (P7_Test P7 pthread dl rt)

