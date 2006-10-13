
add_subdirectory( pics ) 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )


########### next target ###############


########### next target ###############

set(themestandard_PART_SRCS themestandard.cpp wndicon.cpp wndstatus.cpp)

set(ksplashstandard_PART_SRCS themestandardlib.cpp ${themestandard_PART_SRCS})

kde4_automoc(ksplashstandard ${ksplashstandard_PART_SRCS})

kde4_add_plugin(ksplashstandard ${ksplashstandard_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} ksplashstandard)

target_link_libraries(ksplashstandard ${KDE4_KDEUI_LIBS} ${KDE4_KDEFX_LIBS} ksplashthemes)

install(TARGETS ksplashstandard  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES Theme.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksplash/Themes/Standard )
install( FILES ksplashstandard.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

