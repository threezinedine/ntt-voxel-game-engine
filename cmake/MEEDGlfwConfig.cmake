if (TARGET MEEDGlfw)
    return()
endif()

set(MEEDGLFW_DIR "${EXTERNALS_DIR}/glfw")
add_subdirectory("${MEEDGLFW_DIR}" "${CMAKE_BINARY_DIR}/packages/glfw")

set(GLAD_DIR "${EXTERNALS_DIR}/glad")

file(
    GLOB
    GLAD_SOURCES
    "${GLAD_DIR}/src/glad.c"
)

add_library(glad STATIC ${GLAD_SOURCES})
target_include_directories(glad PUBLIC "${GLAD_DIR}/include")

find_package(OpenGL REQUIRED)

add_library(MEEDGlfw INTERFACE)
target_link_libraries(MEEDGlfw INTERFACE glfw glad OpenGL::GL)