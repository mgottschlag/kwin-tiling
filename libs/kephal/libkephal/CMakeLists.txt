FIND_PACKAGE (Qt4 REQUIRED)
SET (QT_USE_QTDBUS 1)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. .. ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/ ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS (-DSCREENS_FACTORY=libkephal_factory)
ADD_DEFINITIONS (-DOUTPUTS_FACTORY=libkephal_factory)
ADD_DEFINITIONS (-DCONFIGURATIONS_FACTORY=libkephal_factory)
ADD_DEFINITIONS (-DNO_KDE)

SET (libkephal_SRCS
        libkephal.cpp
        ../kephal/screens.cpp
        ../screens/simplescreen.cpp
        ../screens/dbus/dbusscreens.cpp
        ../screens/output/outputscreens.cpp
        ../outputs/simpleoutput.cpp
        ../outputs/dbus/dbusoutputs.cpp
        ../outputs/desktopwidget/desktopwidgetoutputs.cpp
        ../kephal/outputs.cpp
        ../kephal/configurations.cpp
        ../configurations/dbus/dbusconfigurations.cpp
        ../configurations/noconfigurations.cpp)
QT4_WRAP_CPP (libkephal_SRCS
        ../kephal/kephal.h
        ../kephal/screens.h
        ../screens/simplescreen.h
        ../screens/dbus/dbusscreens.h
        ../screens/output/outputscreens.h
        ../outputs/simpleoutput.h
        ../outputs/dbus/dbusoutputs.h
        ../outputs/desktopwidget/desktopwidgetoutputs.h
        ../kephal/outputs.h
        ../kephal/configurations.h
        ../configurations/dbus/dbusconfigurations.h
        ../configurations/noconfigurations.h)
QT4_GENERATE_DBUS_INTERFACE(../kded_kephal/dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml screens_interface)
QT4_GENERATE_DBUS_INTERFACE(../kded_kephal/dbus/dbusapi_outputs.h org.kde.Kephal.Outputs.xml)
QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Outputs.xml outputs_interface)
QT4_GENERATE_DBUS_INTERFACE(../kded_kephal/dbus/dbusapi_configurations.h org.kde.Kephal.Configurations.xml)
QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Configurations.xml configurations_interface)
ADD_LIBRARY (kephal SHARED ${libkephal_SRCS})
TARGET_LINK_LIBRARIES (kephal ${QT_LIBRARIES})

INSTALL (TARGETS kephal DESTINATION lib)
INSTALL (FILES 
        ../kephal/screens.h
        ../kephal/outputs.h
        ../kephal/kephal.h
        ../kephal/configurations.h
        ../kephal/kephal_export.h
        DESTINATION include/kephal)
INSTALL (FILES FindKephal.cmake DESTINATION share/apps/cmake/modules)
