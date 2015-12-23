set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCES "*.cpp")

find_path(SOCK libsock.a HINTS ../../corelibs/libsock NO_DEFAULT_PATH)
find_path(PLUGIN libplugin.a HINTS ../../corelibs/libplugin NO_DEFAULT_PATH)

if(${SOCK} STREQUAL "SOCK-NOTFOUND")
  message(FATAL_ERROR "libsock not found")
endif()

if(${PLUGIN} STREQUAL "PLUGIN-NOTFOUND")
  message(FATAL_ERROR "libplugin not found")
endif()

include_directories(
    .
	./event
    ${SOCK}/include
    ${PLUGIN}/include
)

add_executable(BakaBot ${SOURCES})
target_link_libraries(BakaBot ${PLUGIN}/libplugin.a ${SOCK}/libsock.a pthread dl)

install(TARGETS BakaBot RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../run)