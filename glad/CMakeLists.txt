cmake_minimum_required(VERSION 3.17.3)
project(glad)

add_library(glad glad.h glad.c)

target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")