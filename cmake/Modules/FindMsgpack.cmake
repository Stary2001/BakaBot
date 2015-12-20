include(LibFindMacros)
libfind_pkg_check_modules(Msgpack_PKGCONF msgpack)

find_path(Msgpack_INCLUDE_DIR
  NAMES msgpack.h
  PATHS ${Msgpack_PKGCONF_INCLUDE_DIRS}
)

find_library(Msgpack_LIBRARY
  NAMES msgpack
  PATHS ${Msgpack_PKGCONF_LIBRARY_DIRS}
)

set(Msgpack_PROCESS_INCLUDES Msgpack_INCLUDE_DIR)
set(Msgpack_PROCESS_LIBS Msgpack_LIBRARY)
libfind_process(Msgpack)
