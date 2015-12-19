set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCES "*.cpp")

find_package(Jsoncpp)

include_directories(
    .
	./event
	$ENV{COREPATH}/include/plugin
	$ENV{COREPATH}/include/sock
	${Jsoncpp_INCLUDE_DIRS}
)

add_executable(BakaBot ${SOURCES})
target_link_libraries(BakaBot $ENV{COREPATH}/lib/libplugin.a $ENV{COREPATH}/lib/libsock.a pthread dl ${Jsoncpp_LIBRARIES})
