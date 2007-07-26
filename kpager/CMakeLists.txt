

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(kpager_SRCS 
   desktop.cpp 
   kpager.cpp 
   config.cpp 
   windowdrag.cpp
   ksharedpixmap.cpp
   main.cpp )
qt4_add_dbus_adaptor( kpager_SRCS org.kde.KPager.xml kpager.h KPagerMainWindow )

kde4_add_executable(kpager ${kpager_SRCS})

target_link_libraries(kpager  ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kpager DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kpager.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

install( FILES org.kde.KPager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
kde4_install_icons( ${ICON_INSTALL_DIR}  )

