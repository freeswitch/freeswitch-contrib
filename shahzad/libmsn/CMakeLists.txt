CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(libmsn)

# uncomment the following line to enable the inbox feature. This will break the API. Beware.
#set(LIBMSN_INBOX_URL_ENABLED true)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
# find directories for openssl
find_package(OpenSSL REQUIRED)

set (LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OPENSSL_INCLUDE_DIR})
add_subdirectory(msn)

if(NOT WIN32)
add_subdirectory(msntest)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libmsn.pc.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/libmsn.pc
               @ONLY )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/libmsn.pc
         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig
         COMPONENT Devel )
endif(NOT WIN32)
