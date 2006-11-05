


########### next target ###############

set(kcm_arts_PART_SRCS arts.cpp krichtextlabel.cpp )

kde4_automoc(${kcm_arts_PART_SRCS})

kde4_add_ui3_files(kcm_arts_PART_SRCS generaltab.ui hardwaretab.ui )

kde4_add_plugin(kcm_arts ${kcm_arts_PART_SRCS})


target_link_libraries(kcm_arts  ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} kmid )

install(TARGETS kcm_arts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES arts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )


