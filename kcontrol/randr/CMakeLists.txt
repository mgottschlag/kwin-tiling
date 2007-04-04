include_directories( ${X11_Xrandr_INCLUDE_PATH}  )


add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(randrinternal_PART_SRCS ktimerdialog.cpp randr.cpp)

set(kcm_randr_PART_SRCS krandrmodule.cpp ${randrinternal_PART_SRCS})

kde4_automoc(${kcm_randr_PART_SRCS})

kde4_add_plugin(kcm_randr ${kcm_randr_PART_SRCS})


target_link_libraries(kcm_randr  ${KDE4_KDEUI_LIBS} ${X11_Xrandr_LIB} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_randr  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############


########### next target ###############

set(krandrtray_SRCS 
   main.cpp 
   krandrtray.cpp 
   krandrapp.cpp 
   krandrpassivepopup.cpp ${randrinternal_PART_SRCS})

kde4_automoc(${krandrtray_SRCS})

kde4_add_executable(krandrtray ${krandrtray_SRCS})

target_link_libraries(krandrtray  ${KDE4_KUTILS_LIBS} ${X11_Xrandr_LIB} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS krandrtray DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES krandrtray.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES randr.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )
