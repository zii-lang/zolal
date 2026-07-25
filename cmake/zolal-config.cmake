include("cmake/zolal-version.cmake")

set(ZOLAL_VERSION_MAJOR ${zolal_version_major})
set(ZOLAL_VERSION_MINOR ${zolal_version_minor})
set(ZOLAL_VERSION_PATCH ${zolal_version_patch})

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/include/zolal/Config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/include/zolal/Config.hpp
)