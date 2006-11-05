include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )


########### next target ###############


########### next target ###############


set(ksplashdefault_PART_SRCS main.cpp ${themedefault_PART_SRCS})


kde4_automoc(${ksplashdefault_PART_SRCS})

kde4_add_plugin(ksplashdefault ${ksplashdefault_PART_SRCS})


target_link_libraries(ksplashdefault  ${KDE4_KDEUI_LIBS} ksplashthemes )

install(TARGETS ksplashdefault  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ksplashdefault.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


