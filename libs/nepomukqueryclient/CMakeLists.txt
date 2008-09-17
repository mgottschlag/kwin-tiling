project(nepomukqueryclient)

# need 2.6 for set_source_files_properties
cmake_minimum_required(VERSION 2.6)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}
  ${nepomukquery_SOURCE_DIR}
  )

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})# -fPIC)

set(nepomukqueryclient_SRC
  queryserviceclient.cpp
)

if(BUILDING_WORKSPACE_SEPARATELY)
  find_file(QUERY_INTERFACE
    org.kde.nepomuk.Query.xml
    PATHS ${XDG_DATA_DIRS} ${DBUS_INTERFACES_INSTALL_DIR}
    PATH_SUFFIXES / /dbus-1/interfaces/
    )
  if(NOT QUERY_INTERFACE)
    message(FATAL_ERROR "Could not find org.kde.nepomuk.Query.xml. Is kdebase-runtime installed?")
  endif(NOT QUERY_INTERFACE)

  find_file(QUERYSERVICE_INTERFACE
    org.kde.nepomuk.QueryService.xml
    PATHS ${XDG_DATA_DIRS} ${DBUS_INTERFACES_INSTALL_DIR}
    PATH_SUFFIXES / /dbus-1/interfaces/
    )
  if(NOT QUERYSERVICE_INTERFACE)
    message(FATAL_ERROR "Could not find org.kde.nepomuk.QueryService.xml. Is kdebase-runtime installed?")
  endif(NOT QUERYSERVICE_INTERFACE)
else(BUILDING_WORKSPACE_SEPARATELY)
  set(QUERY_INTERFACE ${nepomuk_interfaces_SOURCE_DIR}/org.kde.nepomuk.Query.xml)
  set(QUERYSERVICE_INTERFACE ${nepomuk_interfaces_SOURCE_DIR}/org.kde.nepomuk.QueryService.xml)
endif(BUILDING_WORKSPACE_SEPARATELY)

set_source_files_properties(
  ${QUERYSERVICE_INTERFACE}
  PROPERTIES INCLUDE "querymetatype.h")

set_source_files_properties(
  ${QUERY_INTERFACE}
  PROPERTIES INCLUDE "result.h")

qt4_add_dbus_interface(nepomukqueryclient_SRC
  ${QUERYSERVICE_INTERFACE}
  queryserviceinterface)
qt4_add_dbus_interface(nepomukqueryclient_SRC
  ${QUERY_INTERFACE}
  queryinterface)

kde4_add_library(nepomukqueryclient SHARED ${nepomukqueryclient_SRC})

set_target_properties(nepomukqueryclient PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(nepomukqueryclient
  nepomukquery
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${SOPRANO_LIBRARIES}
  )

install(TARGETS nepomukqueryclient DESTINATION ${LIB_INSTALL_DIR})

install(FILES
  queryserviceclient.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)
