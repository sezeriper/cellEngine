project(cellEngine)

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

add_executable(demo examples/game_of_life.cpp)

target_compile_options(demo PRIVATE /W4)

target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include")
target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_include_directories(demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glad")

target_link_libraries(demo glad glfw glm)
