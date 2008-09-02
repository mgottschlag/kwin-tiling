FIND_PACKAGE (Qt4 REQUIRED)
SET (QT_USE_QTDBUS 1)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. .. ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

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
        ../screens/screens.cpp
        ../screens/simplescreen.cpp
        ../screens/output/outputscreens.cpp
        ../screens/configuration/configurationscreens.cpp
        dbus/dbusapi_screens.cpp
        ../outputs/simpleoutput.cpp
        ../outputs/desktopwidget/desktopwidgetoutputs.cpp
        ../outputs/outputs.cpp
        ../outputs/backendoutputs.cpp
        ../outputs/xrandr/xrandroutputs.cpp
        dbus/dbusapi_outputs.cpp
        dbus/dbusapi_configurations.cpp
        ../configurations/configurations.cpp
        ../configurations/backendconfigurations.cpp
        ../configurations/xml/xmlconfigurations.cpp)
QT4_WRAP_CPP (kephald_SRCS
        ../xml/xmltype.h
        ../xml/configurations_xml.h
        ../xrandr12/randrscreen.h
        ../xrandr12/randroutput.h
        ../xrandr12/randrcrtc.h
        kephald.h
        ../kephal/kephal.h
        ../kephal/screens.h
        ../screens/simplescreen.h
        ../screens/output/outputscreens.h
        ../screens/configuration/configurationscreens.h
        dbus/dbusapi_screens.h
        ../outputs/simpleoutput.h
        ../outputs/desktopwidget/desktopwidgetoutputs.h
        ../kephal/outputs.h
        ../outputs/backendoutputs.h
        ../outputs/xrandr/xrandroutputs.h
        dbus/dbusapi_outputs.h
        dbus/dbusapi_configurations.h
        ../kephal/configurations.h
        ../configurations/backendconfigurations.h
        ../configurations/xml/xmlconfigurations.h)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml
        dbus/dbusapi_screens.h DBusAPIScreens)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_outputs.h org.kde.Kephal.Outputs.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Outputs.xml
        dbus/dbusapi_outputs.h DBusAPIOutputs)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_configurations.h org.kde.Kephal.Configurations.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Configurations.xml
        dbus/dbusapi_configurations.h DBusAPIConfigurations)
ADD_EXECUTABLE (kephald ${kephald_SRCS})
TARGET_LINK_LIBRARIES (kephald ${QT_LIBRARIES})

INSTALL (TARGETS kephald DESTINATION bin)
