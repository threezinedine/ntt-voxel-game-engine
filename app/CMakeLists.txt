cmake_minimum_required(VERSION 3.20)
project(MEEDApp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(MEEDConfigure)

MEEDSetup()

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
        -sFULL_ES3=1
        --preload-file ${CMAKE_SOURCE_DIR}/public/shaders/@shaders/
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