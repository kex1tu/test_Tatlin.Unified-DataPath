cmake_minimum_required(VERSION 3.14)

project(Tatlin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_SOURCES
    src/FileTape.cpp
    src/TapeSorter.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pedantic-errors -Wall -Wextra -Werror -gdwarf-4 -std=c++20")

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_LSAN "Enable LeakSanitizer" OFF)

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

if(ENABLE_LSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -g")
endif()

add_executable(${PROJECT_NAME} src/main.cpp ${LIB_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)

find_package(GTest REQUIRED)
add_executable(unit_tests
    tests/FileTape_test.cpp
    tests/TapeSorter_test.cpp
    tests/TapeConfig_test.cpp
    ${LIB_SOURCES}
)
target_include_directories(unit_tests PRIVATE .)
target_link_libraries(unit_tests GTest::gtest_main GTest::gtest pthread)
enable_testing()
add_test(NAME FileTapeTests COMMAND unit_tests)

