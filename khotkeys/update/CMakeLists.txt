include_directories( ${CMAKE_SOURCE_DIR}/khotkeys/shared/ )


########### install files ###############

set(khotkeys_update_SRCS update.cpp ${libshared_SRCS})

kde4_automoc(${khotkeys_update_SRCS})

kde4_add_executable( khotkeys_update ${khotkeys_update_SRCS})

target_link_libraries( khotkeys_update ${KDE4_KIO_LIBS} )
if(X11_XTest_FOUND)
  target_link_libraries( khotkeys_update ${X11_XTest_LIB} )
endif(X11_XTest_FOUND)

install(TARGETS khotkeys_update DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )
