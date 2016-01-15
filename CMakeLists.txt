cmake_minimum_required (VERSION 3.0)
if (${CMAKE_VERSION} VERSION_LESS 3.1 AND NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
project (BakaBot)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

add_subdirectory(src)
