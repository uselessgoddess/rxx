cmake_minimum_required(VERSION 3.22)
project(rxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

include(CTest)

# if(BUILD_TESTING)
#     set(ANYANY_TEST ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_anyany.cpp)
#     file(GLOB ANYANY_EXAMPLES REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/examples/*)
# endif()
add_executable(rxx ${CMAKE_CURRENT_SOURCE_DIR}/src/rxx.hpp ${RXX_TEST} ${RXX_EXAMPLES})
set_property(TARGET rxx PROPERTY CMAKE_CXX_EXTENSIONS OFF)
set_property(TARGET rxx PROPERTY LINKER_LANGUAGE CXX)

add_library(rxxlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/rxx.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/simply.hpp)
set_property(TARGET rxxlib PROPERTY CMAKE_CXX_EXTENSIONS OFF)

# if(BUILD_TESTING)
#     source_group(Examples FILES ${ANYANY_EXAMPLES})
#     target_include_directories(anyany PRIVATE examples)
#     source_group(Tests FILES ${ANYANY_TEST})
#     source_group(Include FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/anyany.hpp)
#     add_subdirectory(tests)
# endif()


add_executable(main main.cpp)
target_link_libraries(main INTERFACE rxxlib)

set_target_properties(rxx PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
