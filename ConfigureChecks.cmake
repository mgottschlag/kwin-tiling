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

find_program(PROGRAM_XRDB xrdb)
GET_FILENAME_COMPONENT(XBINDIR ${PROGRAM_XRDB} PATH)
GET_FILENAME_COMPONENT(ABSXBINDIR ${XBINDIR} ABSOLUTE)
GET_FILENAME_COMPONENT(XROOTDIR ${ABSXBINDIR} PATH)
set(XLIBDIR "${XROOTDIR}/lib/X11")

check_function_exists(getpassphrase HAVE_GETPASSPHRASE)
check_function_exists(vsyslog HAVE_VSYSLOG)

check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)     # ksmserver, ksplashml, sftp

macro_bool_to_01(FONTCONFIG_FOUND HAVE_FONTCONFIG) # kcontrol/{fonts,kfontinst}
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL) # kwin
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM) # kwin, ksplash
macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST) # khotkeys, kxkb, kdm
macro_bool_to_01(X11_Xcomposite_FOUND HAVE_XCOMPOSITE) # kicker, kwin
macro_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR) # many uses
macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE) # kwin
macro_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES) # klipper, kicker, kwin
macro_bool_to_01(X11_Xinerama_FOUND HAVE_XINERAMA)
macro_bool_to_01(X11_Xrandr_FOUND HAVE_XRANDR) # kwin
macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER) # kcontrol/style, kicker
macro_bool_to_01(X11_Xxf86misc_FOUND HAVE_XF86MISC) # kdesktop and kcontrol/lock
macro_bool_to_01(X11_dpms_FOUND HAVE_DPMS) # kdesktop
macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC) # kwin
