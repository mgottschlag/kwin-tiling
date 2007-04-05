
add_subdirectory( pics ) 
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### next target ###############

set(kcm_ksplashthemes_PART_SRCS installer.cpp main.cpp )

kde4_automoc(${kcm_ksplashthemes_PART_SRCS})

kde4_add_plugin(kcm_ksplashthemes ${kcm_ksplashthemes_PART_SRCS})


target_link_libraries(kcm_ksplashthemes  ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kcm_ksplashthemes  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ksplashthememgr.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

