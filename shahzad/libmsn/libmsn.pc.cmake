prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_PREFIX@/lib@LIB_SUFFIX@
includedir=@CMAKE_INSTALL_PREFIX@/include

Name: libmsn
Description: MSN protocol implementation
Version: 0.4.0

Requires: libssl
Libs: -L${libdir} -lmsn
Cflags: -I${includedir}
