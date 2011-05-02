cmake_minimum_required(VERSION 2.6)
project(popt)

set(POPT_VERSION_MAJOR 1)
set(POPT_VERSION_MINOR 17)
set(POPT_VERSION "${POPT_VERSION_MAJOR}.${POPT_VERSION_MINOR}")

include_directories(${PROJECT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=gnu99")
endif()

#add_subdirectory(po)
#add_subdirectory(auto)

include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckFunctionExists)

check_function_exists(CFPreferencesCopyAppValue HAVE_CFPREFERENCESCOPYAPPVALUE)
check_function_exists(dcgettext HAVE_DCGETTEXT)

check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(float.h HAVE_FLOAT_H)
check_include_files(fnmatch.h HAVE_FNMATCH_H)

check_function_exists(geteuid HAVE_GETEUID)
check_function_exists(gettext HAVE_GETTEXT)
check_function_exists(getuid HAVE_GETUID)

check_include_files(glob.h HAVE_GLOB_H)
check_function_exists(iconv HAVE_ICONV)

check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(langinfo.h HAVE_LANGINFO_H)
check_include_files(libintl.h HAVE_LIBINTL_H)
check_include_files(mcheck.h HAVE_MCHECK_H)
check_include_files(memory.h HAVE_MEMORY_H)

check_function_exists(mtrace HAVE_MTRACE)
check_function_exists(setregid HAVE_SETREGID)
check_function_exists(srandom HAVE_SRANDOM)

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)

check_function_exists(stpcpy HAVE_STPCPY)
check_function_exists(strerror HAVE_STRERROR)

check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)

check_function_exists(vasprintf HAVE_VASPRINTF)
check_function_exists(__secure_getenv HAVE___SECURE_GETENV)

set(libpopt_src popt.c poptparse.c poptconfig.c popthelp.c poptint.c)
set(libpopt_headers popt.h)

set(LINK_FLAGS "-no-undefined ${LINK_FLAGS}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${PROJECT_BINARY_DIR}/config.h)
include_directories(${PROJECT_BINARY_DIR}/)

add_definitions("-DHAVE_CONFIG_H")

set(prefix "") #FIXME
set(popt_sysconfdir "${prefix}/etc")
set(popt_package "popt")
set(popt_package_name "popt")

add_library(popt STATIC ${libpopt_src} ${libpopt_headers})
#add_library(popt SHARED ${libpopt_src} ${libpopt_headers})

#set_target_properties(popt PROPERTIES LANGUAGE CXX)
#set_target_properties(popt PROPERTIES LANGUAGE C)

set_target_properties(popt PROPERTIES PUBLIC_HEADER ${libpopt_headers})


########### tests ###############

set(test1_SRCS test1.c)
add_executable(test1 ${test1_SRCS})
target_link_libraries(test1 popt)

set(test2_SRCS test2.c)
add_executable(test2 ${test2_SRCS})
target_link_libraries(test2 popt)

set(tdict_SRCS tdict.c)
add_executable(tdict ${tdict_SRCS})
target_link_libraries(tdict popt)

########### install files ###############

install(TARGETS popt
          ARCHIVE       DESTINATION lib
          PUBLIC_HEADER DESTINATION include)


