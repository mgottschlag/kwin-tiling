set(soliddevice_engine_SRCS
    soliddeviceengine.cpp
    devicesignalmapper.cpp
    devicesignalmapmanager.cpp
    hddtemp.cpp
    soliddeviceservice.cpp
    soliddevicejob.cpp
)

kde4_add_plugin(plasma_engine_soliddevice ${soliddevice_engine_SRCS})
target_link_libraries(plasma_engine_soliddevice ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBRARY})

install(TARGETS plasma_engine_soliddevice DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-soliddevice.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES soliddevice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services )

