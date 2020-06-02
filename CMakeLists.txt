cmake_minimum_required(VERSION 3.17.3)
project(cellEngine)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

add_executable(game_of_life examples/game_of_life.cpp)

target_compile_features(game_of_life PUBLIC cxx_std_17)
target_compile_options(game_of_life PRIVATE /W4)

target_include_directories(game_of_life PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(game_of_life PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include")
target_include_directories(game_of_life PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_include_directories(game_of_life PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glad")

target_link_libraries(game_of_life glad glfw glm)