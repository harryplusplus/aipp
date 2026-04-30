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

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9)
FetchContent_MakeAvailable(ftxui)

add_executable(aipp src/main.cpp)
target_include_directories(aipp PRIVATE src)
target_link_libraries(aipp PRIVATE CLI11::CLI11 ftxui::component ftxui::screen)
target_compile_definitions(aipp PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
