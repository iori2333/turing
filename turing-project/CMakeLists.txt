add_subdirectory(parser)
add_subdirectory(simulator)

add_executable(turing main.cpp)
target_link_libraries(turing parser simulator)
target_include_directories(turing PUBLIC parser simulator)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
