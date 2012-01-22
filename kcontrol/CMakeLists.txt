
macro_optional_find_package(Freetype)
macro_log_feature(FREETYPE_FOUND "FreeType" "A font rendering engine" "http://www.freetype.org" FALSE "" "Needed to build kfontinst, a simple font installer.")


set(libkxftconfig_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/fonts/kxftconfig.cpp )


if( X11_Xrandr_FOUND )
   add_subdirectory( randr )
endif(X11_Xrandr_FOUND )

if(X11_Xkb_FOUND)
    add_subdirectory( keyboard )
endif(X11_Xkb_FOUND)

if(NOT WIN32)
add_subdirectory( bell )
add_subdirectory( input )
add_subdirectory( access )
add_subdirectory( screensaver )
add_subdirectory( dateandtime )
add_subdirectory( autostart )
endif(NOT WIN32)

add_subdirectory( launch )
add_subdirectory( colors )
add_subdirectory( krdb )
add_subdirectory( style )
add_subdirectory( desktoptheme )
add_subdirectory( standard_actions )
add_subdirectory( keys )
add_subdirectory( workspaceoptions )

# TODO needs porting
#add_subdirectory( smartcard )

add_subdirectory( hardware )
add_subdirectory( desktoppaths )

if( FREETYPE_FOUND )
    if( WIN32 OR FONTCONFIG_FOUND )
        add_subdirectory( fonts )
    endif( WIN32 OR FONTCONFIG_FOUND )
endif( FREETYPE_FOUND )
if(FONTCONFIG_FOUND AND FREETYPE_FOUND AND NOT WIN32)
    add_subdirectory( kfontinst )
endif(FONTCONFIG_FOUND AND FREETYPE_FOUND AND NOT WIN32)
