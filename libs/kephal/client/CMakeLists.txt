include_directories(${kephal_SOURCE_DIR}/kephal)

ADD_DEFINITIONS (-DSCREENS_FACTORY=libkephal_factory)
ADD_DEFINITIONS (-DOUTPUTS_FACTORY=libkephal_factory)
ADD_DEFINITIONS (-DCONFIGURATIONS_FACTORY=libkephal_factory)

SET (kephalshared_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/simplescreen.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/screens.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/desktopwidgetscreens.cpp
   )
   set(unused_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/outputs.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/configurations.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/simpleoutput.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../kephal/desktopwidgetoutputs.cpp
   )

set(libkephal_SRCS
   ${kephalshared_SRCS}
   libkephal.cpp
   )
set(more_unused_SRCS
   dbusconfigurations.cpp
   dbusoutputs.cpp
   dbusscreens.cpp
   noconfigurations.cpp
   )

# generate dbus-interface from class and add the generated sources
#QT4_GENERATE_DBUS_INTERFACE(../service/dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
#QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml screens_interface)
#QT4_GENERATE_DBUS_INTERFACE(../service/dbus/dbusapi_outputs.h org.kde.Kephal.Outputs.xml)
#QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Outputs.xml outputs_interface)
#QT4_GENERATE_DBUS_INTERFACE(../service/dbus/dbusapi_configurations.h org.kde.Kephal.Configurations.xml)
#QT4_ADD_DBUS_INTERFACE(libkephal_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Configurations.xml configurations_interface)

KDE4_ADD_LIBRARY(kephal SHARED ${libkephal_SRCS})

TARGET_LINK_LIBRARIES (kephal ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

set_target_properties(kephal PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

#   INSTALL_RPATH_USE_LINK_PATH true
#   )

INSTALL (TARGETS kephal EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL (FILES FindKephal.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
