FIND_PACKAGE (Qt4)
SET (QT_USE_QTDBUS 1)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

SET (kephald_SRCS kephald.cpp ../screens/desktopwidget/desktopwidgetscreens.cpp
        ../screens/screens.cpp ../screens/screen.cpp ../screens/simplescreen.cpp
        dbus/dbusapi_screens.cpp)
QT4_WRAP_CPP (kephald_SRCS kephald.h ../screens/desktopwidget/desktopwidgetscreens.h
        ../screens/screens.h ../screens/screen.h ../screens/simplescreen.h
        dbus/dbusapi_screens.h)
QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
QT4_ADD_DBUS_ADAPTOR (kephald_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml
        dbus/dbusapi_screens.h DBusAPIScreens)
ADD_EXECUTABLE (kephald ${kephald_SRCS})
TARGET_LINK_LIBRARIES (kephald ${QT_LIBRARIES} kephalxml)

INSTALL (TARGETS kephald DESTINATION bin)
