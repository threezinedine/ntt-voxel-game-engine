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
    PRIVATE 
    MEEDEngine
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE 
    ${COMMON_DEFINITIONS}
)