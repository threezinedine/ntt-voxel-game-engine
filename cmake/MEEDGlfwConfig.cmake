if (TARGET MEEDGlfw)
    return()
endif()

set(MEEDGLFW_DIR "${EXTERNALS_DIR}/glfw")
add_subdirectory("${MEEDGLFW_DIR}" "${CMAKE_BINARY_DIR}/packages/glfw")

set(GLEW_DIR "${EXTERNALS_DIR}/glew")
add_subdirectory("${GLEW_DIR}" "${CMAKE_BINARY_DIR}/packages/glew")

find_package(OpenGL REQUIRED)

add_library(MEEDGlfw INTERFACE)
target_link_libraries(MEEDGlfw INTERFACE glfw libglew_static OpenGL::GL)