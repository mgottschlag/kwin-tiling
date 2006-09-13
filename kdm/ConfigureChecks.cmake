include(CheckCSourceRuns)

check_struct_member("struct passwd" "pw_expire" "pwd.h" HAVE_STRUCT_PASSWD_PW_EXPIRE)
check_struct_member("struct utmp" "ut_user" "utmp.h" HAVE_STRUCT_UTMP_UT_USER)

check_include_files(lastlog.h HAVE_LASTLOG_H)

check_symbol_exists(sysinfo "sys/sysinfo.h" HAVE_SYSINFO)

check_function_exists(initgroups HAVE_INITGROUPS)
check_function_exists(mkstemp HAVE_MKSTEMP)
check_function_exists(getusershell HAVE_GETUSERSHELL)

check_c_source_runs("
#include <errno.h>
#include <unistd.h>
int main()
{
	setlogin(0);
	return errno == ENOSYS;
}
" HAVE_SETLOGIN)

# for config-kdm.h
check_function_exists(seteuid HAVE_SETEUID)

# for environ in config-kdm.h
check_include_files(crt_externs.h HAVE_CRT_EXTERNS_H)
check_function_exists(_NSGetEnviron HAVE_NSGETENVIRON)

find_library(UTIL_LIBRARIES util)
mark_as_advanced(UTIL_LIBRARIES)

macro_push_required_vars()
set(CMAKE_REQUIRED_LIBRARIES ${UTIL_LIBRARIES})
check_function_exists(setusercontext HAVE_SETUSERCONTEXT)
check_function_exists(login_getclass HAVE_LOGIN_GETCLASS)
check_function_exists(auth_timeok HAVE_AUTH_TIMEOK)
if (PAM_FOUND)
	set(CMAKE_REQUIRED_LIBRARIES ${PAM_LIBRARIES})
	check_function_exists(pam_getenvlist HAVE_PAM_GETENVLIST)
endif (PAM_FOUND)
macro_pop_required_vars()

macro(define_library LIB FN)
	set(varname ${FN}_in_${LIB})
	string(TOUPPER ${LIB}_LIBRARIES libname)
	check_library_exists(${LIB} ${FN} "" ${varname})
	set(${libname})
	if (${varname})
		set(${libname} ${LIB})
	endif (${varname})
endmacro(define_library)

define_library(s authenticate)
define_library(posix4 sched_yield)
define_library(socket connect)
define_library(resolv dn_expand)

# for genkdmconf; this is TODO
#if (EXISTS /etc/ttys)
#	set(BSD_INIT 1)
#	check_function_exists(getttyent HAVE_GETTTYENT)
#endif (EXISTS /etc/ttys)

if (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD)
	set(HAVE_UTMPX)
else (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD)
	check_function_exists(getutxent HAVE_UTMPX)
endif (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD)
set(BSD_UTMP)
if (NOT HAVE_UTMPX)
	check_function_exists(getutent have_getutent)
	if (NOT have_getutent)
		set(BSD_UTMP 1)
	endif (NOT have_getutent)
endif (NOT HAVE_UTMPX)

check_function_exists(arc4random HAVE_ARC4RANDOM)
if (NOT HAVE_ARC4RANDOM)
	# assume that /dev/random is non-blocking if /dev/urandom does not exist
	if (EXISTS /dev/urandom)
		set(DEV_RANDOM "\"/dev/urandom\"")
	else (EXISTS /dev/urandom)
		if (EXISTS /dev/random)
			set(DEV_RANDOM "\"/dev/random\"")
		endif (EXISTS /dev/random)
	endif (EXISTS /dev/urandom)
endif (NOT HAVE_ARC4RANDOM)

option(KDE4_RPCAUTH "Use Sun's secure RPC for Xauth cookies in KDM" OFF)
if (KDE4_RPCAUTH)
	find_path(RPC_INCLUDE_DIR rpc/rpc.h)
	if (RPC_INCLUDE_DIR)
		set(RPCAUTH_FOUND TRUE)
	else (RPC_INCLUDE_DIR)
		message(STATUS "Sun's secure RPC header was not found")
	endif (RPC_INCLUDE_DIR)
endif (KDE4_RPCAUTH)
mark_as_advanced(RPC_INCLUDE_DIR)
macro_bool_to_01(RPCAUTH_FOUND SECURE_RPC)

option(KDE4_KRB5AUTH "Use Sun's secure RPC for Xauth cookies in KDM" OFF)
if (KDE4_KRB5AUTH)
	find_library(KRB5_LIBRARIES krb5)
	if (KRB5_LIBRARIES)
		find_path(KRB5_INCLUDE_DIR krb5/krb5.h)
		if (KRB5_INCLUDE_DIR)
				set(KRB5AUTH_FOUND TRUE)
		else (KRB5_INCLUDE_DIR)
				message(STATUS "KDE4_KRB5AUTH requires Kerberos5 header files.
Due to a problem with X includes you probably have to run \"ln -s . krb5\"
in the directory where the krb5.h include resides to make things actually work.")
		endif (KRB5_INCLUDE_DIR)
	endif (KRB5_LIBRARIES)
	mark_as_advanced(KRB5_INCLUDE_DIR KRB5_LIBRARIES)
	macro_bool_to_01(KRB5AUTH_FOUND K5AUTH)
endif (KDE4_KRB5AUTH)

if (X11_Xdmcp_FOUND)
	macro_push_required_vars()
	set(CMAKE_REQUIRED_LIBRARIES ${X11_LIBRARIES})
	check_function_exists(XdmcpWrap HASXDMAUTH)
	macro_pop_required_vars()
endif (X11_Xdmcp_FOUND)

option(KDE4_KERBEROS4 "Compile KDM with Kerberos v4 support" OFF)
if (KDE4_KERBEROS4)
	find_path(KRB4_INCLUDE_DIR krb.h)
	find_library(KRB4_LIBRARY krb)
	find_library(DES_LIBRARY des)
	if (KRB4_INCLUDE_DIR AND KRB4_LIBRARY)
		set(KERBEROS 1)
		set(KRB4_LIBRARIES ${KRB4_LIBRARY} ${DES_LIBRARY} ${RESOLV_LIBRARIES})
	endif (KRB4_INCLUDE_DIR AND KRB4_LIBRARY)
	option(KDE4_AFS "Compile KDM with AFS support" OFF)
	if (KDE4_AFS)
		find_path(AFS_INCLUDE_DIR kafs.h)
		find_library(AFS_LIBRARY kafs)
		if (AFS_INCLUDE_DIR AND AFS_LIBRARY)
			set(AFS 1)
			set(KRB4_LIBRARIES ${KRB4_LIBRARIES} ${AFS_LIBRARY})
		endif (AFS_INCLUDE_DIR AND AFS_LIBRARY)
	endif (KDE4_AFS)
endif (KDE4_KERBEROS4)

option(KDE4_XDMCP "Build KDM with XDMCP support" ON)
if (KDE4_XDMCP AND X11_Xdmcp_FOUND)
	set(XDMCP 1)
endif (KDE4_XDMCP AND X11_Xdmcp_FOUND)

option(KDE4_KDM_XCONSOLE "Build KDM with built-in xconsole" OFF)
macro_bool_to_01(KDE4_KDM_XCONSOLE WITH_KDM_XCONSOLE)
