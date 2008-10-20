# Project Needs a name ofcourse
project(desktopthemedetails)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(kcmdesktopthemedetails_SRCS desktopthemedetails.cpp)
kde4_add_ui_files(kcmdesktopthemedetails_SRCS DesktopThemeDetails.ui)

kde4_add_plugin(kcm_desktopthemedetails ${kcmdesktopthemedetails_SRCS})
target_link_libraries(kcm_desktopthemedetails ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KUTILS_LIBRARY} plasma ${KDE4_KIO_LIBS})

install(TARGETS kcm_desktopthemedetails  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES desktopthemedetails.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES themeitems DESTINATION ${DATA_INSTALL_DIR}/desktopthemedetails )
