project(cellEngine)

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

add_library(cellEngine INTERFACE)

target_link_libraries(cellEngine INTERFACE glad)
target_link_libraries(cellEngine INTERFACE glfw)
target_link_libraries(cellEngine INTERFACE glm)


target_include_directories(cellEngine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(cellEngine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_include_directories(cellEngine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include")