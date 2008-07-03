FIND_PACKAGE (Qt4)
SET (QT_USE_QTDBUS 1)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

SET (kephal_SRCS kephalapp.cpp)
QT4_WRAP_CPP (kephal_SRCS kephalapp.h)
#QT4_ADD_DBUS_INTERFACE(kephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/../kephald/org.kde.Kephal.Screens.xml screens_interface)
ADD_EXECUTABLE (kephalapp ${kephal_SRCS})
TARGET_LINK_LIBRARIES (kephalapp ${QT_LIBRARIES} kephal)

