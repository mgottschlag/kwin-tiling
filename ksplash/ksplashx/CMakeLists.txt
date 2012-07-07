project( ksplashx )

add_subdirectory( scale )
add_subdirectory( utils )
add_subdirectory( themes )

include_directories(${PNG_INCLUDE_DIR}  # Deprecated in CMake 2.8.7
                    ${PNG_INCLUDE_DIRS} # Added in CMake 2.8.7
                    ${JPEG_INCLUDE_DIR}
                   )

include (CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("	#ifdef __SUNPRO_CC
					#define __asm__ asm
				#endif
				int main() { __asm__(\"pxor %mm0, %mm0\") ; }" X86_MMX_FOUND)

########### next target ###############

set( ksplashx_SRCS
    main.cpp
    qglobal.cpp
    qjpegio.cpp
    qpngio.cpp
    qimage.cpp
    qsize.cpp
    pixmap.cpp
    qcolor.cpp
    qcolor_x11.cpp
    qcolor_p.cpp
    x11_defs.cpp
    defs.cpp
    splash.cpp
    qrect.cpp
    kcpuinfo.cpp
    scale.cpp )

# this is intentionally disabled using the '2' since it doesn't work on x86_64,
# so this needs somebody who knows assembly enough to fix it
if( X86_MMX_FOUND2 )
    set( ksplashx_SRCS ${ksplashx_SRCS}
        asm_scale.S )
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asm_scale.o
        COMMAND gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/asm_scale.S 
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/asm_scale.S )
    set( ksplashx_EXTRA_SRCS ${CMAKE_CURRENT_BINARY_DIR}/asm_scale.o )
    ADD_DEFINITIONS( -DHAVE_X86_MMX )
endif( X86_MMX_FOUND2 )

add_executable( ksplashx ${ksplashx_SRCS} ${ksplashx_EXTRA_SRCS} )
target_link_libraries( ksplashx ${X11_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
if( X11_Xinerama_FOUND )
    target_link_libraries( ksplashx ${X11_Xinerama_LIB} )
endif( X11_Xinerama_FOUND )
install( TARGETS ksplashx ${INSTALL_TARGETS_DEFAULT_ARGS} )
