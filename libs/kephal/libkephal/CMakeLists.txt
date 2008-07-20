FIND_PACKAGE (Qt4 REQUIRED)
SET (QT_USE_QTDBUS 1)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS (-DSCREENS_FACTORY=screens_factory)

SET (libkephal_SRCS libkephal.cpp ../screens/screens.cpp ../screens/screen.cpp
        ../screens/simplescreen.cpp
        ../screens/dbus/dbusscreens.cpp)
QT4_WRAP_CPP (libkephal_SRCS ../screens/screens.h ../screens/simplescreen.h
        ../screens/screen.h ../screens/dbus/dbusscreens.h)
QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/../kephald/org.kde.Kephal.Screens.xml screens_interface)
ADD_LIBRARY (kephal SHARED ${libkephal_SRCS})
INSTALL (TARGETS kephal DESTINATION lib)
INSTALL (FILES ../screens/screens.h ../screens/screen.h DESTINATION include/kephal)
INSTALL (FILES FindKephal.cmake DESTINATION share/apps/cmake/modules)