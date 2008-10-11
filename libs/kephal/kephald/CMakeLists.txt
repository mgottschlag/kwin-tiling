FIND_PACKAGE (Qt4 REQUIRED)
SET (QT_USE_QTDBUS 1)

INCLUDE (${QT_USE_FILE})

FIND_PACKAGE (KDE4 REQUIRED)
INCLUDE (KDE4Defaults)

INCLUDE_DIRECTORIES (. .. ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/ ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})


SET (kephald_SRCS
        kephald.cpp
        ../xml/xmltype.cpp
        ../xml/configurations_xml.cpp
        ../xml/xmlnodehandler.cpp
        ../xrandr12/randr.cpp
        ../xrandr12/randrscreen.cpp
        ../xrandr12/randroutput.cpp
        ../xrandr12/randrcrtc.cpp
        ../xrandr12/randrmode.cpp
        ../xrandr12/randrdisplay.cpp
        ../kephal/screens.cpp
        ../screens/simplescreen.cpp
        ../screens/output/outputscreens.cpp
        ../screens/configuration/configurationscreens.cpp
        dbus/dbusapi_screens.cpp
        ../outputs/simpleoutput.cpp
        ../outputs/desktopwidget/desktopwidgetoutputs.cpp
        ../kephal/outputs.cpp
        ../outputs/backendoutputs.cpp
        ../outputs/xrandr/xrandroutputs.cpp
        dbus/dbusapi_outputs.cpp
        dbus/dbusapi_configurations.cpp
        ../kephal/configurations.cpp
        ../configurations/backendconfigurations.cpp
        ../configurations/xml/xmlconfigurations.cpp)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml
        dbus/dbusapi_screens.h DBusAPIScreens)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_outputs.h org.kde.Kephal.Outputs.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Outputs.xml
        dbus/dbusapi_outputs.h DBusAPIOutputs)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_configurations.h org.kde.Kephal.Configurations.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Configurations.xml
        dbus/dbusapi_configurations.h DBusAPIConfigurations)
        
KDE4_ADD_PLUGIN (kded_kephal ${kephald_SRCS})
TARGET_LINK_LIBRARIES (kded_kephal ${KDE4_KDECORE_LIBS} ${QT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrandr_LIB})

INSTALL (TARGETS kded_kephal DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (FILES
        kded_kephal.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kded)
