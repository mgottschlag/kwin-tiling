project(plasma-dataengine-nowplaying)

set(nowplaying_engine_SRCS
    nowplayingengine.cpp
    player.cpp
    playerfactory.cpp
    dbuswatcher.cpp
    pollingwatcher.cpp
    mpris.cpp
)

set(juk_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.juk.player.xml)
if(EXISTS ${juk_xml})
   QT4_ADD_DBUS_INTERFACE(nowplaying_engine_SRCS ${juk_xml} juk_interface)
   set(nowplaying_engine_SRCS ${nowplaying_engine_SRCS} juk.cpp)
   set(JUK_FOUND ON)
   message(STATUS "Found JuK D-Bus interface specification: the nowplaying dataengine will support JuK")
else(EXISTS ${juk_xml})
   message(STATUS "JuK D-Bus interface specification NOT found: the nowplaying dataengine will NOT support JuK")
endif(EXISTS ${juk_xml})
macro_log_feature(JUK_FOUND "JuK" "Jukebox and music manager for KDE" "http://developer.kde.org/~wheeler/juk.html" FALSE "" "Support for JuK in the Now Playing data engine")

set(amarok_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.amarok.player.xml)
if(EXISTS ${amarok_xml})
   QT4_ADD_DBUS_INTERFACE(nowplaying_engine_SRCS ${amarok_xml} amarok_interface)
   set(nowplaying_engine_SRCS ${nowplaying_engine_SRCS} amarok.cpp)
   set(AMAROK_FOUND ON)
   message(STATUS "Found Amarok D-Bus interface specification: the nowplaying dataengine will support Amarok")
else(EXISTS ${amarok_xml})
   message(STATUS "Amarok D-Bus interface specification NOT found: the nowplaying dataengine will NOT support Amarok")
endif(EXISTS ${amarok_xml})
macro_log_feature(AMAROK_FOUND "Amarok" "Music player for KDE" "http://amarok.kde.org/" FALSE "" "Support for Amarok in the Now Playing data engine")

macro_optional_find_package(GLIB2)
if(GLIB2_FOUND)
   macro_optional_find_package(Xmms)
   macro_log_feature(XMMS_FOUND "XMMS" "X MultiMedia System developement libraries" "http://www.xmms.org" FALSE "" "Support for XMMS in the Now Playing data engine")
   if(XMMS_FOUND)
      include_directories(${XMMS_INCLUDE_DIRS})
      include_directories(${GLIB2_INCLUDE_DIR})
      set(nowplaying_engine_SRCS ${nowplaying_engine_SRCS} xmms.cpp)
      message(STATUS "Found XMMS: the nowplaying dataengine will support XMMS 1.x")
   else(XMMS_FOUND)
      message(STATUS "Could NOT find XMMS: the nowplaying dataengine will NOT support XMMS 1.x")
   endif(XMMS_FOUND)
endif(GLIB2_FOUND)

QT4_ADD_DBUS_INTERFACE(nowplaying_engine_SRCS mpris_interface.xml mpris_interface)

configure_file(config-nowplaying.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-nowplaying.h)

kde4_add_plugin(plasma_engine_nowplaying ${nowplaying_engine_SRCS})
target_link_libraries(plasma_engine_nowplaying ${KDE4_KIO_LIBS} plasma ${QT_QTGUI_LIBRARY})
if(XMMS_FOUND)
   target_link_libraries(plasma_engine_nowplaying ${XMMS_LDFLAGS})
endif(XMMS_FOUND)

install(TARGETS plasma_engine_nowplaying DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-nowplaying.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

