set (CMAKE_CXX_STANDARD 11)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

file(GLOB_RECURSE SOURCES "*.cpp")

find_path(SOCK NAMES libsock.a sock.lib HINTS ../../corelibs/libsock NO_DEFAULT_PATH)
find_path(PLUGIN NAMES libplugin.a plugin.lib HINTS ../../corelibs/libplugin NO_DEFAULT_PATH)
find_package(Jsoncpp REQUIRED)
find_package(CURL REQUIRED)
find_package(GnuTLS REQUIRED)
find_package(Lua REQUIRED)  

if(${SOCK} STREQUAL "SOCK-NOTFOUND")
  message(FATAL_ERROR "libsock not found")
endif()

if(${PLUGIN} STREQUAL "PLUGIN-NOTFOUND")
    message(FATAL_ERROR "libplugin not found")
endif()

include_directories(
    .
    ./event
    ./commands
    ${SOCK}/include
    ${PLUGIN}/include
    ${Jsoncpp_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${GNUTLS_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
)

add_executable(BakaBot ${SOURCES})
if(UNIX)
  target_link_libraries(BakaBot ${GNUTLS_LIBRARY} ${PLUGIN}/libplugin.a ${SOCK}/libsock.a pthread dl ${Jsoncpp_LIBRARY} ${CURL_LIBRARY} ${LUA_LIBRARIES})
elseif(WIN32)
  target_link_libraries(BakaBot ws2_32.lib ${GNUTLS_LIBRARY} ${PLUGIN}/plugin.lib ${SOCK}/sock.lib ${Jsoncpp_LIBRARY} ${CURL_LIBRARY} ${LUA_LIBRARIES})
endif()

install(TARGETS BakaBot RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../run)

if(WIN32)
	install(FILES ${CMAKE_BINARY_DIR}/src/\${BUILD_TYPE}/bakabot.lib DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
endif()
