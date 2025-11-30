cmake_minimum_required(VERSION 3.20)

file(
    GLOB 
    EXAMPLE_SOURCES
    "*.c"
)

foreach (EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_FILE_NAME ${EXAMPLE_SOURCE} NAME_WE)

    set(EXAMPLE_FILE_PROJECT ${EXAMPLE_FILE_NAME})

    add_executable(
        ${EXAMPLE_FILE_PROJECT}
        ${EXAMPLE_SOURCE}
    )

    target_link_libraries(
        ${EXAMPLE_FILE_PROJECT}
        PUBLIC
        MEEDEngine
    )

    target_compile_definitions(
        ${EXAMPLE_FILE_PROJECT}
        PUBLIC 
        ${COMMON_DEFINITIONS}
    )
endforeach()