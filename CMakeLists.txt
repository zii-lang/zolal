cmake_minimum_required(VERSION 3.20)
project(zolal LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
        tomlPlusPlus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(tomlplusplus)

option(ZOLAL_ENABLE_TESTS "Enable unit tests" OFF)

set(LIB_SOURCES
        src/DirectiveLexer.cpp
        src/Preprocessor.cpp
        src/Manifest.cpp
)

add_library(zolallib STATIC ${LIB_SOURCES})
target_include_directories(zolallib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(zolallib PUBLIC tomlplusplus::tomlplusplus)

add_executable(zolal src/main.cpp)
target_link_libraries(zolal PRIVATE zolallib)

if(ZOLAL_ENABLE_TESTS)
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.17.0.zip
    )
    FetchContent_MakeAvailable(googletest)
  endif()

  include(GoogleTest)
  enable_testing()

  file(GLOB TEST_SOURCES "tests/*.cpp")
  add_executable(zpp_tests ${TEST_SOURCES})
  target_link_libraries(zpp_tests PRIVATE zolallib GTest::gtest GTest::gtest_main)
  gtest_discover_tests(zpp_tests)
endif()
