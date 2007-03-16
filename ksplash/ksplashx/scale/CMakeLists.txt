########### next target ###############

set( ksplashx_scale_SRCS
    main.cpp )

kde4_add_executable( ksplashx_scale ${ksplashx_scale_SRCS})
target_link_libraries( ksplashx_scale ${KDE4_KDECORE_LIBS})
install( TARGETS ksplashx_scale DESTINATION ${BIN_INSTALL_DIR} )
