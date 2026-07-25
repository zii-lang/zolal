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
option(ZOLAL_ENABLE_TESTS "Enable unit tests" OFF)
option(ZOLAL_BUILD_EXECUTEABLE "Build zolal executable" ON)

FetchContent_Declare(
	tomlPlusPlus
	GIT_REPOSITORY
	https://github.com/marzer/tomlplusplus.git
	GIT_TAG
	v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)
list(APPEND zolal_libraries "tomlplusplus::tomlplusplus")

file(GLOB_RECURSE zolal_library_sources
	lib/*.cpp
)

if(ZOLAL_BUILD_SHARED)
	list(APPEND zolal_build_targets "shared")
endif()

if(ZOLAL_BUILD_STATIC)
	list(APPEND zolal_build_targets "static")
endif()

message(STATUS "")
message(STATUS "Library Name   : ${zolal_libname}")
message(STATUS "CXX Compiler   : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Targets  : ${zolal_build_targets}")
message(STATUS "Link Libraries : ${zolal_libraries}")
message(STATUS "")

if(ZOLAL_BUILD_SHARED)
	add_library(zolal-shared SHARED ${zolal_library_sources})
	set_target_properties(zolal-shared
		PROPERTIES
			VERSION ${zolal_version}
			SOVERSION ${zolal_version_major}
			OUTPUT_NAME ${zolal_libname}
			ARCHIVE_OUTPUT_NAME zolal-import
	)
	target_link_libraries(zolal-shared PRIVATE ${zolal_libraries})
	target_include_directories(zolal-shared
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:${zolal_install_incdir}>
	)
	install(
		TARGETS zolal-shared
		EXPORT zolal
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

	add_library(zolal::shared ALIAS zolal-shared)
endif()

if(ZOLAL_BUILD_STATIC)
	add_library(zolal-static STATIC ${zolal_library_sources})
	set_property(TARGET zolal-static PROPERTY OUTPUT_NAME ${zolal_libname})
	set_property(TARGET zolal-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(zolal-static
		PRIVATE
			${zolal_defines}
			ZOLAL_STATIC_LIB
	)
	target_compile_options(zolal-static
		PRIVATE
			${zolal_cxxflags}
			${zolal_cxxflags_static}
	)
	target_link_libraries(zolal-static PRIVATE ${zolal_libraries})
	target_include_directories(zolal-static
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:${zolal_install_incdir}>
	)
	install(
		TARGETS zolal-static
		EXPORT zolal
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

	add_library(zolal::static ALIAS zolal-static)
endif()

# Install include directories
install(
	DIRECTORY include/zolal
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(ZOLAL_BUILD_EXECUTEABLE)

	FetchContent_Declare(
		argparse
		GIT_REPOSITORY
		https://github.com/p-ranav/argparse.git
		GIT_TAG
		master
	)
	FetchContent_MakeAvailable(argparse)
	if(ZOLAL_BUILD_STATIC)
		list(APPEND zolal_exe_libs zolal::static)
	elseif(ZOLAL_BUILD_SHARED)
		list(APPEND zolal_exe_libs zolal::shared)
	else()
		message(ERROR
			"Unable to build executable, either static or shared library should be built."
		)
	endif()
	list(APPEND zolal_exe_libs "argparse")

	message(STATUS "")
	message(STATUS "Executable Name : zolal")
	message(STATUS "CXX Compiler    : ${CMAKE_CXX_COMPILER}")
	message(STATUS "Link Libraries  : ${zolal_exe_libs}")
	message(STATUS "")

	add_executable(zolal src/Zolal.cpp)
	target_link_libraries(zolal PRIVATE ${zolal_exe_libs})
	target_include_directories(zolal
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:${zolal_install_incdir}>
	)
	install(TARGETS zolal EXPORT zolal DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()

if(ZOLAL_ENABLE_TESTS)
	find_package(GTest QUIET)
	if(NOT GTest_FOUND)
		FetchContent_Declare(
			googletest
			URL
			https://github.com/google/googletest/archive/refs/tags/release-1.17.0.zip
		)
		FetchContent_MakeAvailable(googletest)
	endif()

	include(GoogleTest)
	enable_testing()

	file(GLOB TEST_SOURCES "tests/*.cpp")
	add_executable(zpp_tests ${TEST_SOURCES})
	target_link_libraries(zpp_tests
		PRIVATE
			${zolal_library}
			GTest::gtest
			GTest::gtest_main
	)
	gtest_discover_tests(zpp_tests)
endif()
