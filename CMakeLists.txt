cmake_minimum_required(VERSION 3.17.3)
project(cellEngine)

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

add_library(
    cellEngine
    cellEngine.hpp
)