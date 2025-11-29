cmake_minimum_required(VERSION 3.20)
project(MEEDEngineTests LANGUAGES CXX C)

file(
    GLOB 
    TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/**/**/*.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${TEST_SOURCE_FILES}
)

MEEDFindPackage(googletest "Dependencies")

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    MEEDEngine
    googletest
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)