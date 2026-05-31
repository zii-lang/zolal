cmake_minimum_required(VERSION 3.20)

project(libzolal LANGUAGES CXX)

# +-------------------------------------------------------------------------+
# | unorderd_map.contains requires c++ standard 20, do not change this.     |
# +-------------------------------------------------------------------------+
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(CheckIncludeFiles)
include(GNUInstallDirs)
include("cmake/zolal-version.cmake")

set(zolal_libname "zolal")

set(zolal_cxxflags "")
set(zolal_cxxflags_static "")
set(zolal_cxxflags_dynamic "")
set(zolal_libraries "")
set(zolal_defines "")
set(zolal_library "")

set(zolal_install_objdir "${CMAKE_INSTALL_LIBDIR}")
set(zolal_install_incdir "${CMAKE_INSTALL_INCLUDEDIR}")

# -- Options --
option(ZOLAL_BUILD_SHARED "Build shared library" ON)
option(ZOLAL_BUILD_STATIC "Build static library" ON)
option(ZOLAL_BUILD_OBJECT "Build object library" OFF) # TODO: NOT YET IMPLEMENTED
option(ZOLAL_ENABLE_TESTS "Enable unit tests" OFF)

FetchContent_Declare(
        tomlPlusPlus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(tomlplusplus)
list(APPEND zolal_libraries "tomlplusplus::tomlplusplus")

set(LIB_SOURCES
        src/DirectiveLexer.cpp
        src/Preprocessor.cpp
        src/Manifest.cpp
)

if(ZOLAL_BUILD_SHARED)
	list(APPEND zolal_build_targets "shared")
endif()

if(ZOLAL_BUILD_STATIC)
	list(APPEND zolal_build_targets "static")
endif()

if(ZOLAL_BUILD_OBJECT)
	list(APPEND zolal_build_targets "object")
endif()

message(STATUS "")
message(STATUS "Library Name   : ${zolal_libname}")
message(STATUS "CXX Compiler   : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Targets  : ${zolal_build_targets}")
message(STATUS "Link Libraries : ${zolal_libraries}")
message(STATUS "")

if(ZOLAL_BUILD_SHARED)
	add_library(zolal-shared SHARED ${LIB_SOURCES})
	set_target_properties(zolal-shared PROPERTIES VERSION ${zolal_version} SOVERSION ${zolal_version_major} OUTPUT_NAME ${zolal_libname})
	target_link_libraries(zolal-shared PRIVATE ${zolal_libraries})
	target_include_directories(zolal-shared PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${zolal_install_incdir}>
	)
	install(TARGETS zolal-shared EXPORT zolal ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

	if("${zolal_library}" STREQUAL "")
		set(zolal_library "zolal-shared")
	endif()
endif()

if(ZOLAL_BUILD_STATIC)
	add_library(zolal-static STATIC ${LIB_SOURCES})
	set_property(TARGET zolal-static PROPERTY OUTPUT_NAME ${zolal_libname})
	set_property(TARGET zolal-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(zolal-static PRIVATE ${zolal_defines} ZOLAL_STATIC_LIB)
	target_compile_options(zolal-static PRIVATE ${zolal_cxxflags} ${zolal_cxxflags_static})
	target_link_libraries(zolal-static PRIVATE ${zolal_libraries})
	target_include_directories(zolal-static PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${zolal_install_incdir}>
	)
	install(TARGETS zolal-static EXPORT zolal ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

	if("${zolal_library}" STREQUAL "")
		set(zolal_library "zolal-static")
	endif()
endif()

if(ZOLAL_BUILD_OBJECT)
	add_library(zolal-object OBJECT src/static.cpp)
	set_property(TARGET zolal-object PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(zolal-object PRIVATE ${zolal_defines})
	target_compile_options(zolal-object PRIVATE ${zolal_cxxflags} ${zolal_cxxflags_static})
	target_link_libraries(zolal-object PRIVATE ${zolal_libraries})
	target_include_directories(zolal-object PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${zolal_install_incdir}>
	)
	if(CMAKE_GENERATOR MATCHES "^Visual Studio.*$")
		set(zolal-object-static "${CMAKE_CURRENT_BINARY_DIR}/zolal-object.dir/$<CONFIG>/static${CMAKE_CXX_OUTPUT_EXTENSION}")
	else()
		set(zolal-object-static "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/zolal-object.dir/src/static.cpp${CMAKE_CXX_OUTPUT_EXTENSION}")
	endif()
	set(zolal-object-out "${CMAKE_CURRENT_BINARY_DIR}/${zolal_libname}${CMAKE_C_OUTPUT_EXTENSION}")
	add_custom_command(OUTPUT ${zolal-object-out} DEPENDS zolal-object COMMAND "${CMAKE_COMMAND}" -E copy "${zolal-object-static}" "${zolal-object-out}")
	add_custom_target(zolal-object-target ALL DEPENDS ${zolal-object-out})

	install(FILES ${zolal-object-static} DESTINATION ${zolal_install_objdir} RENAME ${zolal_libname}${CMAKE_C_OUTPUT_EXTENSION})

	if("${zolal_library}" STREQUAL "")
		set(zolal_library "zolal-object")
	endif()
endif()

# Install include directories
install(
    DIRECTORY include/zolal
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

add_executable(zolal src/main.cpp)
target_link_libraries(zolal PRIVATE ${zolal_library})
target_include_directories(zolal PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${zolal_install_incdir}>
)
install(TARGETS zolal EXPORT zolal DESTINATION ${CMAKE_INSTALL_BINDIR})

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
  target_link_libraries(zpp_tests PRIVATE ${zolal_library} GTest::gtest GTest::gtest_main)
  gtest_discover_tests(zpp_tests)
endif()
