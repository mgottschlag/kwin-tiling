
add_subdirectory( default ) 
add_subdirectory( pics ) 



########### next target ###############

set(kcm_locale_PART_SRCS 
   main.cpp 
   toplevel.cpp 
   kcmlocale.cpp 
   klocalesample.cpp 
   localenum.cpp 
   localemon.cpp 
   localetime.cpp 
   localeother.cpp )

kde4_automoc(${kcm_locale_PART_SRCS})
kde4_add_ui_files(kcm_locale_PART_SRCS toplevelbase.ui language.ui )

kde4_add_plugin(kcm_locale ${kcm_locale_PART_SRCS})


target_link_libraries(kcm_locale  ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_locale  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES language.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

