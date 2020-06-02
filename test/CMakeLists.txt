cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_STANDARD 11)


project(test_chayes)

find_package(GTest)


file(GLOB C_CORE_SRCS ${PROJECT_SOURCE_DIR}/../third_party/c_core/src/*.c)

add_executable(test_chayes
    main.cpp
    test_parser.cpp
    test_checker.cpp
    test_core.cpp
    test_shim.c
    urc_util.cpp
    ../src/hayes_parser.c
    ../src/hayes_checker.c
    ../src/chayes.c
    ../src/shim.c
    
    ${C_CORE_SRCS}
)
target_compile_definitions(
     test_chayes
     PUBLIC UNITTEST)


target_include_directories(
    test_chayes
    PUBLIC ${GTSEST_INCLUDE_DIRS}
    PUBLIC ../src/
    PUBLIC ../third_party/c_core/src
)

target_link_libraries(
    test_chayes
    ${GTEST_LIBRARIES} 
    pthread
    rt
)

