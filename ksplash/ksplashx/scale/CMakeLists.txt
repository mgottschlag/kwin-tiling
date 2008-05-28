########### next target ###############

set( ksplashx_scale_SRCS
    main.cpp )

kde4_add_executable( ksplashx_scale ${ksplashx_scale_SRCS})
target_link_libraries( ksplashx_scale ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install( TARGETS ksplashx_scale ${INSTALL_TARGETS_DEFAULT_ARGS} )
