cmake_minimum_required(VERSION 3.20)
project(MEEDEngine)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake)
include(MEEDConfigure)

MEEDSetup()
MEEDDetectPlatform()

file(
    GLOB 
    SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/**/*.c
)

file(
    GLOB 
    HEADER_FILES 
    ${CMAKE_SOURCE_DIR}/include/MEEDEngine/*.h
    ${CMAKE_SOURCE_DIR}/include/MEEDEngine/**/*.h
)

MEEDFindPackage(MEEDpybind11 "Dependencies")

set(PROJECT_SOURCE_FILES ${SRC_FILES} ${HEADER_FILES})
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(PROJECT_DEFINITIONS ${PLATFORM_DEFINITION})
set(PROJECT_PRECOMPILED_HEADER ${CMAKE_SOURCE_DIR}/include/MEEDEngine/platforms/common.h)
set(PROJECT_LIBRARIES)

# ================ Engine Lib ================

add_library(
    ${PROJECT_NAME}
    STATIC
    ${PROJECT_SOURCE_FILES}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_DEFINITIONS}
)

target_precompile_headers(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_PRECOMPILED_HEADER}
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_LIBRARIES}
)

# ================ Examples ================
add_subdirectory("examples")

# ================ Engine Binding Lib ================
set(BINDIND_PROJECT "${PROJECT_NAME}Binding")
set(BINDING_SOURCE_FILES ${CMAKE_SOURCE_DIR}/tmp/binding.cpp)

add_library(
    ${BINDIND_PROJECT}
    STATIC
    ${BINDING_SOURCE_FILES} 
)

target_link_libraries(
    ${BINDIND_PROJECT}
    PUBLIC
    ${PROJECT_NAME}
    MEEDpybind11
)