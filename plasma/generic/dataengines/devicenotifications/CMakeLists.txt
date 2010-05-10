set(device_notifications_engine_SRCS
    devicenotificationsengine.cpp
)

qt4_add_dbus_adaptor( device_notifications_engine_SRCS org.kde.DeviceNotifications.xml devicenotificationsengine.h DeviceNotificationsEngine )

kde4_add_plugin(plasma_engine_devicenotifications ${device_notifications_engine_SRCS})

target_link_libraries(plasma_engine_devicenotifications ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS})

install(TARGETS plasma_engine_devicenotifications DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-devicenotifications.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
