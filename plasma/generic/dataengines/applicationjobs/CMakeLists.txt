project(kuiserver_engine)

#include (KDE4Defaults)
#include (MacroLibrary)

set(kuiserver_engine_SRCS
    kuiserverengine.cpp
    jobcontrol.cpp
    jobaction.cpp
)

qt4_add_dbus_adaptor(kuiserver_engine_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobViewV2.xml kuiserverengine.h JobView jobviewadaptor )
qt4_add_dbus_adaptor(kuiserver_engine_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobViewServer.xml kuiserverengine.h KuiserverEngine jobviewserveradaptor )

kde4_add_plugin(plasma_engine_applicationjobs ${kuiserver_engine_SRCS})
target_link_libraries(plasma_engine_applicationjobs ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_applicationjobs DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-applicationjobs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES applicationjobs.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

