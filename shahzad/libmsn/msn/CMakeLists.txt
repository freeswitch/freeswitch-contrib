########### next target ###############

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(msn_STAT_SRCS
    authdata.cpp
    message.cpp
    connection.cpp
    switchboardserver.cpp
    notificationserver.cpp
    util.cpp
    md5.cpp
    soap.cpp
    p2p.cpp
    xmlParser.cpp
    msnobject.cpp
    buddy.cpp
    passport.cpp
)

if(WIN32)
set(msn_STAT_SRCS   ${msn_STAT_SRCS}
                    ${OPENSSL_INCLUDE_DIR}/openssl/applink.c)
endif(WIN32)

set(msn_HEADERS
    authdata.h
    message.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    connection.h
    switchboardserver.h
    notificationserver.h
    util.h
    externals.h
    errorcodes.h
    msn.h
    buddy.h
    passport.h
    sstream_fix.h
    soap.h
    p2p.h
    msnobject.h
    libmsn_export.h)

set(siren_STAT_SRCS
    libsiren/common.cpp
    libsiren/dct4.cpp
    libsiren/decoder.cpp
    libsiren/encoder.cpp
    libsiren/huffman.cpp
    libsiren/rmlt.cpp
)

set(siren_HEADERS
    libsiren/common.h
    libsiren/dct4.h
    libsiren/decoder.h
    libsiren/encoder.h
    libsiren/huffman_consts.h
    libsiren/huffman.h
    libsiren/rmlt.h
    libsiren/siren7.h
)
add_library(msn SHARED ${msn_STAT_SRCS} ${siren_STAT_SRCS})
set_target_properties(msn PROPERTIES VERSION 0.1.0 
                                     SOVERSION 0.1
)

if(NOT WIN32)
    target_link_libraries(msn crypto)
else(NOT WIN32)
    target_link_libraries(msn ${OPENSSL_EAY_LIBRARIES})
endif(NOT WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

########### install files ###############
install(TARGETS msn RUNTIME DESTINATION bin
                    LIBRARY DESTINATION lib${LIB_SUFFIX}
                    ARCHIVE DESTINATION lib${LIB_SUFFIX})
install(FILES ${msn_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/msn)
