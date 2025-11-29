if (TARGET googletest)
    return()
endif()

set(gtest_DIR "${EXTERNALS_DIR}/googletest")
add_subdirectory(${gtest_DIR} ${CMAKE_BINARY_DIR}/packages/googletest)

add_library(googletest INTERFACE)
target_link_libraries(
    googletest
    INTERFACE
    gtest
    gtest_main
    gmock
    gmock_main
)