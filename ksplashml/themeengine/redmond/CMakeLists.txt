
add_subdirectory( pics ) 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )


########### next target ###############

set(ksplashredmond_PART_SRCS previewredmond.cpp themeredmond.cpp ${ksplashthemes_LIB_SRCS})

kde4_automoc(ksplashredmond ${ksplashredmond_PART_SRCS})

kde4_add_plugin(ksplashredmond ${ksplashredmond_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} ksplashredmond)

target_link_libraries(ksplashredmond ${KDE4_KIO_LIBS} ${KDE4_KDEFX_LIBS} ksplashthemes)

install(TARGETS ksplashredmond  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES Theme.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksplash/Themes/Redmond )
install( FILES ksplashredmond.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
