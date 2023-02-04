cmake_minimum_required(VERSION 3.13)

project(CCGEN)
add_executable(ccgen main.c EpmGen.c)
target_include_directories(ccgen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})