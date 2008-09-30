# Project's name
project(plasma-qedje-script)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

# We add our source code here
set(package_SRCS qedje_package.cpp)
set(qedjescript_SRCS qedje_applet.cpp)
kde4_add_ui_files(qedjescript_SRCS qedjeConfig.ui)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_appletscript_qedje ${qedjescript_SRCS})
target_link_libraries(plasma_appletscript_qedje
  plasma ${KDE4_KDEUI_LIBS} ${QEdje_LIBRARIES})


kde4_add_plugin(plasma_package_qedje ${package_SRCS})
target_link_libraries(plasma_package_qedje
  plasma ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})


install(TARGETS plasma_appletscript_qedje
  DESTINATION ${PLUGIN_INSTALL_DIR})

install(TARGETS plasma_package_qedje
  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-appletscript-qedje.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES plasma-package-qedje.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})

