cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE debug)

project(test_chayes)

find_package(GTest)


file(GLOB C_CORE_SRCS ${PROJECT_SOURCE_DIR}/../third_party/c_core/src/*.c)

add_executable(test_chayes
    main.cpp
    test_parser.cpp
    ../src/hayes_parser.c
    ${C_CORE_SRCS}
)

target_include_directories(
    test_chayes
    PUBLIC ${GTSEST_INCLUDE_DIRS}
    PUBLIC ../src/
    ../third_party/c_core/src
)

target_link_libraries(
    test_chayes
    ${GTEST_LIBRARIES} 
    pthread
)

