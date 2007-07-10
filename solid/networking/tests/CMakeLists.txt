set(networkingservice_SRCS networkingservice.cpp)

kde4_add_ui_files(networkingservice_SRCS networkingserviceview.ui)

qt4_add_dbus_interface(networkingservice_SRCS ../org.kde.Solid.Networking.Service.xml serviceinterface)


kde4_add_executable(networkingservice ${networkingservice_SRCS})

target_link_libraries(networkingservice ${KDE4_KIO_LIBS})

