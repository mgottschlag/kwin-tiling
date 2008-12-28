PROJECT (kephald)

INCLUDE_DIRECTORIES (. .. ../kded_kephal ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/ ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS (-DNO_KDE)

SET (kephald_SRCS
        ../xml/xmltype.cpp
        ../xml/configurations_xml.cpp
        ../xml/xmlnodehandler.cpp
        ../xrandr12/randr.cpp
        ../xrandr12/randrscreen.cpp
        ../xrandr12/randroutput.cpp
        ../xrandr12/randrcrtc.cpp
        ../xrandr12/randrmode.cpp
        ../xrandr12/randrdisplay.cpp
        kephald.cpp
        ../kephal/screens.cpp
        ../screens/simplescreen.cpp
        ../screens/output/outputscreens.cpp
        ../screens/configuration/configurationscreens.cpp
        ../kded_kephal/dbus/dbusapi_screens.cpp
        ../outputs/simpleoutput.cpp
        ../outputs/desktopwidget/desktopwidgetoutputs.cpp
        ../kephal/outputs.cpp
        ../outputs/backendoutputs.cpp
        ../outputs/xrandr/xrandroutputs.cpp
        ../kded_kephal/dbus/dbusapi_outputs.cpp
        ../kded_kephal/dbus/dbusapi_configurations.cpp
        ../kephal/configurations.cpp
        ../configurations/backendconfigurations.cpp
        ../configurations/externalconfiguration.cpp
        ../configurations/xml/xmlconfigurations.cpp)
QT4_WRAP_CPP (kephald_SRCS
        ../xml/xmltype.h
        ../xml/configurations_xml.h
        ../xrandr12/randrscreen.h
        ../xrandr12/randroutput.h
        ../xrandr12/randrcrtc.h
        kephald.h
        ../kephal/screens.h
        ../screens/simplescreen.h
        ../screens/output/outputscreens.h
        ../screens/configuration/configurationscreens.h
        ../kded_kephal/dbus/dbusapi_screens.h
        ../outputs/simpleoutput.h
        ../outputs/desktopwidget/desktopwidgetoutputs.h
        ../kephal/outputs.h
        ../outputs/backendoutputs.h
        ../outputs/xrandr/xrandroutputs.h
        ../kded_kephal/dbus/dbusapi_outputs.h
        ../kded_kephal/dbus/dbusapi_configurations.h
        ../kephal/configurations.h
        ../configurations/externalconfiguration.h
        ../configurations/backendconfigurations.h
        ../configurations/xml/xmlconfigurations.h)
QT4_GENERATE_DBUS_INTERFACE(../kded_kephal/dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml
        ../kded_kephal/dbus/dbusapi_screens.h DBusAPIScreens)
QT4_GENERATE_DBUS_INTERFACE(../kded_kephal/dbus/dbusapi_outputs.h org.kde.Kephal.Outputs.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Outputs.xml
        ../kded_kephal/dbus/dbusapi_outputs.h DBusAPIOutputs)
QT4_GENERATE_DBUS_INTERFACE(../kded_kephal/dbus/dbusapi_configurations.h org.kde.Kephal.Configurations.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Configurations.xml
        ../kded_kephal/dbus/dbusapi_configurations.h DBusAPIConfigurations)
        
ADD_EXECUTABLE (kephald ${kephald_SRCS})
TARGET_LINK_LIBRARIES (kephald ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

INSTALL (TARGETS kephald ${INSTALL_TARGETS_DEFAULT_ARGS})
