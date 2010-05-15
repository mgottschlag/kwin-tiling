find_package(KDE4 4.4.59 REQUIRED)
include(MacroLibrary)
include(KDE4Defaults)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
KDE4_NO_ENABLE_FINAL(styles)

include_directories( ${QIMAGEBLITZ_INCLUDES} )

########### next target ###############

set(kcm_desktopstyle_PART_SRCS kcmstyle.cpp)

kde4_add_ui_files(kcm_desktopstyle_PART_SRCS theme.ui )

kde4_add_plugin(kcm_desktopstyle ${kcm_desktopstyle_PART_SRCS})

target_link_libraries(kcm_desktopstyle ${KDE4_KIO_LIBS} ${QIMAGEBLITZ_LIBRARIES} ${X11_LIBRARIES} ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF3_LIBS})

install(TARGETS kcm_desktopstyle  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES desktopstyle.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
