include(UnixAuth)

macro_optional_find_package(XKB) # kxkb, kdm

if (PAM_FOUND)
    set(KDE4_COMMON_PAM_SERVICE "kde" CACHE STRING "The PAM service to use unless overridden for a particular app.")

    macro(define_pam_service APP)
        string(TOUPPER ${APP}_PAM_SERVICE var)
        set(cvar KDE4_${var})
        set(${cvar} "${KDE4_COMMON_PAM_SERVICE}" CACHE STRING "The PAM service for ${APP}.")
        mark_as_advanced(${cvar})
        set(${var} "\"${${cvar}}\"")
    endmacro(define_pam_service)

    macro(install_pam_service APP)
        string(TOUPPER KDE4_${APP}_PAM_SERVICE cvar)
        # XXX: we shouldn't do this if a DESTDIR is set. how is this done with cmake?
        install(CODE "
exec_program(${CMAKE_SOURCE_DIR}/workspace/mkpamserv ARGS ${${cvar}} RETURN_VALUE ret)
if (NOT ret)
    exec_program(${CMAKE_SOURCE_DIR}/workspace/mkpamserv ARGS -P ${${cvar}}-np)
endif (NOT ret)
        ")
    endmacro(install_pam_service)

    define_pam_service(KDM)
    define_pam_service(kcheckpass)
    define_pam_service(kscreensaver)

else (PAM_FOUND)

    macro(install_pam_service APP)
    endmacro(install_pam_service)

endif (PAM_FOUND)

check_function_exists(getpassphrase HAVE_GETPASSPHRASE)
check_function_exists(vsyslog HAVE_VSYSLOG)
