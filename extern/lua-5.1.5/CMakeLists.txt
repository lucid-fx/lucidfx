# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Drahoš, Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( lua C )
cmake_minimum_required ( VERSION 3.22 )
include ( cmake/dist.cmake )
include ( lua )

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=macro-redefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=implicit-function-declaration")
endif()

## CONFIGURATION
# Default configuration (we assume POSIX by default)
set ( LUA_PATH "LUA_PATH" CACHE STRING "Environment variable to use as package.path." )
set ( LUA_CPATH "LUA_CPATH" CACHE STRING "Environment variable to use as package.cpath." )
set ( LUA_INIT "LUA_INIT" CACHE STRING "Environment variable for initial script." )

option ( LUA_ANSI "Use only ansi features." OFF )
option ( LUA_USE_RELATIVE_LOADLIB "Use modified loadlib.c with support for relative paths on posix systems." 
  ON )
set ( LUA_IDSIZE 60 CACHE STRING "gives the maximum size for the description of the source." )
set ( LUA_PROMPT "> " CACHE STRING "Is the default prompt used by stand-alone Lua." )
set ( LUA_PROMPT2 ">> " CACHE STRING "Is the default continuation prompt used by stand-alone Lua." )
set ( LUA_MAXINPUT 512 CACHE STRING "Is the maximum length for an input line in the stand-alone interpreter." )

#2DO: LUAI_* and LUAL_* settings, for now defaults are used.
set ( LUA_DIRSEP "/" )
set ( LUA_MODULE_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX} )
set ( LUA_LDIR ${INSTALL_LMOD} )
set ( LUA_CDIR ${INSTALL_CMOD} )

if ( LUA_USE_RELATIVE_LOADLIB )
  # This will set up relative paths to lib
  string ( REGEX REPLACE "[^!/]+" ".." LUA_DIR "!/${INSTALL_BIN}/" )
else ( )
  # Direct path to installation
  set ( LUA_DIR ${CMAKE_INSTALL_PREFIX} CACHE STRING "Destination from which modules will be resolved. See INSTALL_LMOD and INSTALL_CMOD." )
endif ( )

set ( LUA_PATH_DEFAULT "./?.lua;${LUA_DIR}${LUA_LDIR}/?.lua;${LUA_DIR}${LUA_LDIR}/?/init.lua;./?/init.lua" )
set ( LUA_CPATH_DEFAULT "./?${LUA_MODULE_SUFFIX};${LUA_DIR}${LUA_CDIR}/?${LUA_MODULE_SUFFIX};${LUA_DIR}${LUA_CDIR}/loadall${LUA_MODULE_SUFFIX}" )

# Android specific flags to prevent FORTIFY buffer overflow
if (ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector")
  # Disable Lua assertions that can cause issues on Android
  add_compile_definitions(LUA_USE_APICHECK=0)
  add_compile_definitions("LUAI_ASSERT(x)=((void)0)")
endif()

if ( WIN32 AND NOT CYGWIN )
  # Windows systems
  option ( LUA_WIN "Windows specific build." ON )
  option ( LUA_BUILD_WLUA "Build wLua interpretter without console output." ON )
  option ( LUA_BUILD_AS_DLL "Build Lua library as Dll." ${BUILD_SHARED_LIBS} )

  # Paths (Double escapes needed)
  set ( LUA_DIRSEP "\\\\" ) 
  string ( REPLACE " /" ${LUA_DIRSEP} LUA_DIR "${LUA_DIR}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_LDIR "${LUA_LDIR}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_CDIR "${LUA_CDIR}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_PATH_DEFAULT "${LUA_PATH_DEFAULT}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_CPATH_DEFAULT "${LUA_CPATH_DEFAULT}" )
else ( )
  # Posix systems (incl. Cygwin)
  option ( LUA_USE_POSIX "Use POSIX functionality." ON )
  option ( LUA_USE_DLOPEN "Use dynamic linker to load modules." ON )
  option ( LUA_USE_MKSTEMP "Use mkstep." ON )
  option ( LUA_USE_ISATTY "Use tty." ON )
  option ( LUA_USE_POPEN "Use popen." ON )
  option ( LUA_USE_ULONGJMP "Use ulongjmp" ON )
endif ( )

#if( NOT MSVC )


## SETUP
# Optional libraries
#find_package ( Readline )
#if ( READLINE_FOUND )
#  option ( LUA_USE_READLINE "Use readline in the Lua CLI." ON )
#endif ( )

#find_package ( Curses )
#if ( CURSES_FOUND )
#  option ( LUA_USE_CURSES "Use curses in the Lua CLI." ON )
#endif ( )

# Setup needed variables and libraries
if ( LUA_USE_POSIX )
  # On POSIX Lua links to standard math library "m"
	find_library ( MATH_LIBRARY NAMES m )
	if ( MATH_LIBRARY )
		list ( APPEND LIBS ${MATH_LIBRARY} )
	endif ( )
endif ( )

if ( LUA_USE_DLOPEN )
  # Link to dynamic linker library "dl"
	find_library ( DL_LIBRARY NAMES dl )
	if ( DL_LIBRARY )
		list ( APPEND LIBS ${DL_LIBRARY} )
	endif ( )
endif ( )

if ( LUA_WIN )
  # Add extra rc files to the windows build
  if ( MSVC OR MINGW )
    set ( LUA_DEF src/lua.def )
    set ( LUA_DLL_RC src/lua_dll.rc )
    set ( LUA_RC src/lua.rc )
    set ( LUAC_RC src/luac.rc )
  endif ( )
endif ( )

#if ( LUA_USE_READLINE )
#  # Add readline
#  include_directories ( ${READLINE_INCLUDE_DIR} )
#  list ( APPEND LIBS ${READLINE_LIBRARY} )
#endif ( )
#
#if ( LUA_USE_CURSES )
#  # Add curses
#  include_directories ( ${CURSES_INCLUDE_DIR} )
#  list ( APPEND LIBS ${CURSES_LIBRARY} )
#endif ( )

## SOURCES
# Generate luaconf.h
configure_file ( src/luaconf.h.in ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h )

# Sources and headers
include_directories ( src ${CMAKE_CURRENT_BINARY_DIR} )
set ( SRC_LIB src/lapi.c src/lcode.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c 
  src/lgc.c src/llex.c src/lmem.c src/lobject.c src/lopcodes.c src/lparser.c src/lstate.c 
  src/lstring.c src/ltable.c src/ltm.c src/lundump.c src/lvm.c src/lzio.c src/lauxlib.c 
  src/lbaselib.c src/ldblib.c src/liolib.c src/lmathlib.c src/loslib.c src/ltablib.c 
  src/lstrlib.c src/linit.c src/bit.c )
set ( SRC_LUA src/lua.c )
set ( SRC_LUAC src/luac.c src/print.c )

if ( LUA_USE_RELATIVE_LOADLIB )
  # Use modified loadlib
  list ( APPEND SRC_LIB src/loadlib_rel.c )
else ( )
  list ( APPEND SRC_LIB src/loadlib.c )
endif ( )

## BUILD
# Create dynamic library
add_library ( liblua SHARED ${SRC_LIB} ${LUA_DLL_RC} ${LUA_DEF} )
target_link_libraries ( liblua ${LIBS} )
set_target_properties ( liblua PROPERTIES OUTPUT_NAME lua CLEAN_DIRECT_OUTPUT 1 )
if ( LUA_BUILD_AS_DLL )
  set_target_properties ( liblua PROPERTIES COMPILE_DEFINITIONS LUA_BUILD_AS_DLL )
endif ( )

# Create static library, this is needed to compile luac in the 5.1.x Lua series
add_library ( liblua_static STATIC ${SRC_LIB} )
target_link_libraries ( liblua_static ${LIBS} )

add_executable ( lua ${SRC_LUA} ${LUA_RC} )
target_link_libraries ( lua liblua )

# On windows a variant of the lua interpreter without console output needs to be built
if ( LUA_BUILD_WLUA )
  add_executable ( wlua WIN32 src/wmain.c ${SRC_LUA} ${LUA_RC} )
  target_link_libraries ( wlua liblua )
  install_executable ( wlua )
endif ( )

add_executable ( luac ${SRC_LUAC} ${LUAC_RC} )
target_link_libraries ( luac liblua_static )

install_executable ( lua luac )
install_library ( liblua )
install_data ( README.md COPYRIGHT HISTORY )
install_lua_module ( strict etc/strict.lua )
install_header ( src/lua.h src/lualib.h src/lauxlib.h etc/lua51.hpp ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h )
install_doc ( doc/ )
install_foo ( etc/ )
install_test ( test/ )
install(FILES src/lua.h src/lualib.h src/lauxlib.h DESTINATION include/lua51)


## TESTS
set ( LUA lua )

add_lua_test ( test/bisect.lua )
add_lua_test ( test/cf.lua )
add_lua_test ( test/echo.lua )
add_lua_test ( test/env.lua )
add_lua_test ( test/factorial.lua )
add_lua_test ( test/fib.lua 20 )
add_lua_test ( test/fibfor.lua )
#add_lua_test ( test/globals.lua ) # Requires input
add_lua_test ( test/hello.lua )
file ( READ test/life.lua _data )
# life.lua test, with reduced run-time.
string ( REPLACE "40,20" "20,15" _data "${_data}" )
string ( REPLACE 2000 20 _data "${_data}" )
file ( WRITE ${CMAKE_CURRENT_BINARY_DIR}/test/life-quick.lua "${_data}" )
add_lua_test ( ${CMAKE_CURRENT_BINARY_DIR}/test/life-quick.lua )
#add_lua_test ( test/luac.lua ) # Requires input
add_lua_test ( test/printf.lua )
#add_lua_test ( test/readonly.lua )
#set_property ( TEST readonly PROPERTY PASS_REGULAR_EXPRESSION "cannot redefine global variable `y'" )
add_lua_test ( test/sieve.lua )
add_lua_test ( test/sort.lua )
#add_lua_test ( test/table.lua ) # Requires input
add_lua_test ( test/trace-calls.lua )
add_lua_test ( test/trace-globals.lua )
#add_lua_test ( test/xd.lua ) # Requires input
