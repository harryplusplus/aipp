cmake_minimum_required(VERSION 3.20)
project(
  aipp
  VERSION 0.0.0
  DESCRIPTION "An AI agent harness written in C++"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.6.2)
FetchContent_MakeAvailable(cli11)

find_package(Curses REQUIRED)

include(CTest)

if(BUILD_TESTING)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_executable(aipp_test src/main_test.cc)
  target_link_libraries(aipp_test PRIVATE GTest::gtest GTest::gtest_main)
  target_include_directories(aipp_test PRIVATE src)
  add_test(NAME aipp_test COMMAND aipp_test)
endif()

add_executable(aipp src/main.cc)
target_include_directories(aipp PRIVATE src)
target_link_libraries(aipp PRIVATE CLI11::CLI11 ${CURSES_LIBRARIES})
target_include_directories(aipp PRIVATE ${CURSES_INCLUDE_DIR})
target_compile_definitions(aipp PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
