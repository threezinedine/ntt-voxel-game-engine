cmake_minimum_required(VERSION 3.20)
project(MEEDApp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(MEEDConfigure)

MdSetup()

file(
    GLOB 
    APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(
    ${PROJECT_NAME} 
    ${APP_SOURCES}
)

MEEDFindPackage(MEEDEngine "engine")

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC 
    MEEDEngine
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC 
    ${COMMON_DEFINITIONS}
)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
    ${COMMON_OPTIONS}
)

if (PLATFORM_IS_WEB)
    target_link_options(
        ${PROJECT_NAME}
        PUBLIC
        -sUSE_GLFW=3
        -sUSE_WEBGL2=1
        -sMIN_WEBGL_VERSION=2
        -sMAX_WEBGL_VERSION=2
        -sFULL_ES3=1
        --preload-file ${PROJECT_BASE_DIR}/engine/assets/shaders/web@shaders/
    )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/MEEDApp.js ${CMAKE_SOURCE_DIR}/public/MEEDApp.js
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/MEEDApp.wasm ${CMAKE_SOURCE_DIR}/public/MEEDApp.wasm
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/MEEDApp.data ${CMAKE_SOURCE_DIR}/public/MEEDApp.data
        COMMENT "Copying WebAssembly output to public folder"
    )
endif()