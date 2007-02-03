
project(kpager)

########### next target ###############

set(kpager_SRCS 
   desktop.cpp 
   kpager.cpp 
   config.cpp 
   windowdrag.cpp
   ksharedpixmap.cpp
   main.cpp )
qt4_add_dbus_adaptor( kpager_SRCS org.kde.KPager.xml kpager.h KPagerMainWindow )
qt4_add_dbus_interface( kpager_SRCS ../kdesktop/org.kde.kdesktop.Background.xml kdesktop_background_interface )
kde4_automoc(${kpager_SRCS})

kde4_add_executable(kpager ${kpager_SRCS})

target_link_libraries(kpager  ${KDE4_KDEUI_LIBS} )

install(TARGETS kpager DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kpager.desktop  DESTINATION  ${XDG_APPS_DIR} )

install( FILES org.kde.KPager.xml DESTINATION ${DBUS_INTERFACES_DIR} )
kde4_install_icons( ${ICON_INSTALL_DIR}  )

