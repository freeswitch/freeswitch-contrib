#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.6])
AC_INIT([xmlcdrd], [0.1], [xmlcdr@buta-tech.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([urldecode.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

AC_CHECK_LIB([apreq2], [main])

AC_CHECK_LIB([apr-1], [main])

AC_CHECK_LIB([config], [main])

AC_CHECK_LIB([dl], [main])

AC_CHECK_LIB([fcgi], [main])

AC_CHECK_LIB([pcre], [main])

AC_CHECK_LIB([glib-2.0], [main])

AC_CHECK_LIB([xml2], [main])

AC_CHECK_LIB([freeradius-client], [main])

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stdlib.h string.h syslog.h])

#check for library config scripts
AC_MSG_CHECKING(for apreq2config)
AC_ARG_WITH(apreq2_config,
    AC_HELP_STRING([--with-apreq2-config@<:@=PATH@:>@],[Location of apreq2 configuration script.]),
    [with_apreq2_config="$withval"],[with_apreq2_config="apreq2-config"])
AC_SUBST(with_apreq2_config)

AC_MSG_CHECKING(for aprconfig)
AC_ARG_WITH(apr_config,
    AC_HELP_STRING([--with-apr-config@<:@=PATH@:>@],[Location of apr configuration script.]),
    [with_apr_config="$withval"],[with_apr_config="apr-1-config"])
AC_SUBST(with_apr_config)

AC_MSG_CHECKING(for mysqlconfig)
AC_ARG_WITH(mysql_config,
    AC_HELP_STRING([--with-mysql-config@<:@=PATH@:>@],[Location of mysql configuration script.]),
    [with_mysql_config="$withval"],[with_mysql_config="mysql_config"])
AC_SUBST(with_mysql_config)

AC_MSG_CHECKING(for xml2config)
AC_ARG_WITH(xml2_config,
    AC_HELP_STRING([--with-xml2-config@<:@=PATH@:>@],[Location of libxml2 configuration script.]),
    [with_xml2_config=="$withval"],[with_xml2_config="xml2-config"])
AC_SUBST(with_xml2_config)

AC_MSG_CHECKING(for glib2 flags)
AC_ARG_WITH(glib2_flags,
    AC_HELP_STRING([--with-glib2-flags],[glib2 CFLAGS.]),
    [with_glib2_flags="$withval"],[with_glib2_flags="-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include"])
AC_SUBST(with_glib2_flags)

AC_MSG_CHECKING(for lua includes)
AC_ARG_WITH(lua_includes,
    AC_HELP_STRING([--with-lua-includes],[--with-lua-includes@<:@=PATH@:>@]),
    [with_lua_includes="$withval"],[with_lua_includes="-I/usr/include/lua5.1"])
AC_SUBST(with_lua_includes)

AC_MSG_CHECKING(for lua lib)
AC_ARG_WITH(lua_lib,
    AC_HELP_STRING([--with-lua-lib],[--with-lua-lib@<:@=-llua51@:>@]),
    [with_lua_lib="$withval"],[with_lua_lib="-llua5.1"])
AC_SUBST(with_lua_lib)

AC_MSG_CHECKING(for radius includes)
AC_ARG_WITH(radius_includes,
    AC_HELP_STRING([--with-radius-includes],[--with-radius-includes@<:@=PATH@:>@]),
    [with_radius_includes="$withval"],[with_radius_includes="-I/usr/include"])
AC_SUBST(with_radius_includes)

AC_MSG_CHECKING(for radius lib)
AC_ARG_WITH(radius_lib,
    AC_HELP_STRING([--with-radius-lib],[--with-radius-lib@<:@=-lfreeradius-client@:>@]),
    [with_radius_lib="$withval"],[with_radius_lib="-lfreeradius-client"])
AC_SUBST(with_radius_lib)




#Plugins to build

#mysqlcdr
domysqlcdr=yes

AC_ARG_ENABLE(mysqlcdr,
AS_HELP_STRING([--disable-mysqlcdr], [Disable building of mysqlcdr plugin]),
[if test "$enableval" = "no"; then domysqlcdr="no"; fi],
[
domysqlcdr=yes
]
)

AM_CONDITIONAL(BUILDMYSQLCDR, test x$domysqlcdr = xyes)


#radcdr
doradcdr=yes

AC_ARG_ENABLE(radcdr,
AS_HELP_STRING([--disable-radcdr], [Disable building of radcdr plugin]),
[if test "$enableval" = "no"; then doradcdr="no"; fi],
[
doradcdr=yes
]
)

AM_CONDITIONAL(BUILDRADCDR, test x$doradcdr = xyes)


#luaexec
doluaexec=yes

AC_ARG_ENABLE(luaexec,
AS_HELP_STRING([--disable-luaexec], [Disable building of luaexec plugin]),
[if test "$enableval" = "no"; then doluaexec="no"; fi],
[
doluaexec=yes
]
)

AM_CONDITIONAL(BUILDLUAEXEC, test x$doluaexec = xyes)



# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strchr strdup strstr])

AC_CONFIG_FILES([Makefile
                 plugins/mysqlcdr/Makefile
                 plugins/radcdr/Makefile
                 plugins/luaexec/Makefile])
AC_OUTPUT

