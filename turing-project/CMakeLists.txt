file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.h *.hpp)

add_executable(turing ${SOURCES} ${HEADERS})
target_include_directories(turing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# if gcc < 10, add -fconcepts flag, add macro __turing_legacy__
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
        target_compile_options(turing PUBLIC -fconcepts)
        target_compile_definitions(turing PUBLIC __turing_legacy__)
    endif ()
endif ()
