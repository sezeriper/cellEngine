cmake_minimum_required(VERSION 3.17.3)
project(cellEngine)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

add_executable(game_of_life examples/game_of_life.cpp)

target_include_directories(game_of_life PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(game_of_life PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include")
target_include_directories(game_of_life PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_include_directories(game_of_life PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad")

target_link_libraries(game_of_life glad glfw glm)