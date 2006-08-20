find_package(PAM)

set(UNIXAUTH_LIBRARIES)
set(UNIXAUTH_INCLUDE_DIRS)

if (PAM_FOUND)

    set(HAVE_PAM 1)
    set(UNIXAUTH_LIBRARIES ${PAM_LIBRARIES})
    set(UNIXAUTH_INCLUDE_DIRS ${PAM_INCLUDE_DIR})

else (PAM_FOUND)

    check_function_exists(getspnam found_getspnam)
    if (found_getspnam)
        set(HAVE_GETSPNAM 1)
    else (found_getspnam)
        macro_push_required_vars()
        set(CMAKE_REQUIRED_LIBRARIES -lshadow)
        check_function_exists(getspnam found_getspnam_shadow)
        if (found_getspnam_shadow)
            set(HAVE_GETSPNAM 1)
            set(UNIXAUTH_LIBRARIES shadow)
        else (found_getspnam_shadow)
            set(CMAKE_REQUIRED_LIBRARIES -lgen) # UnixWare
            check_function_exists(getspnam found_getspnam_gen)
            if (found_getspnam_gen)
                set(HAVE_GETSPNAM 1)
                set(UNIXAUTH_LIBRARIES gen)
            endif (found_getspnam_gen)
        endif (found_getspnam_shadow)
        macro_pop_required_vars()
    endif (found_getspnam)

    check_library_exists(crypt crypt "" HAVE_CRYPT)
    if (HAVE_CRYPT)
        set(UNIXAUTH_LIBRARIES ${UNIXAUTH_LIBRARIES} crypt)
        check_include_files(crypt.h HAVE_CRYPT_H)
    endif (HAVE_CRYPT)

    macro_push_required_vars()
    set(CMAKE_REQUIRED_LIBRARIES ${UNIXAUTH_LIBRARIES}) # some older Linux
    check_function_exists(pw_encrypt HAVE_PW_ENCRYPT)
    macro_pop_required_vars()

endif (PAM_FOUND)
