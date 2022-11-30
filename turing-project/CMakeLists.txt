file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.h *.hpp)

add_executable(turing ${SOURCES} ${HEADERS})
target_include_directories(turing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
