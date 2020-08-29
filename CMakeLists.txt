cmake_minimum_required(VERSION 3.17.3)
project(cellEngine)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

add_executable(demo main.cpp)

target_compile_options(demo PRIVATE /W4)

target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include")
target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glad")

target_link_libraries(demo glad glfw glm)