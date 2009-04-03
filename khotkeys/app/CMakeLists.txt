########### next target ###############

set(kded_khotkeys_PART_SRCS 
        kded.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.khotkeys.xml)

qt4_generate_dbus_interface(kded.h org.kde.khotkeys.xml )

kde4_add_plugin(kded_khotkeys ${kded_khotkeys_PART_SRCS})
target_link_libraries(kded_khotkeys khotkeysprivate kworkspace ${KDE4_KDEUI_LIBS})

# I'm not sure if this is needed anymore. Check it
if(X11_XTest_FOUND)
    target_link_libraries(kded_khotkeys ${X11_XTest_LIB})
endif(X11_XTest_FOUND)


########### install files ###############

install(TARGETS kded_khotkeys  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES khotkeys.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/kded)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.khotkeys.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


