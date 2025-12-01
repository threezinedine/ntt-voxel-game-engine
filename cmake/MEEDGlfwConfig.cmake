if (TARGET MEEDGlfw)
    return()
endif()

add_library(MEEDGlfw INTERFACE)

if (NOT PLATFORM_IS_WEB)
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

    target_link_libraries(MEEDGlfw INTERFACE glfw glad OpenGL::GL)
else()
    target_link_libraries(MEEDGlfw INTERFACE)
endif()