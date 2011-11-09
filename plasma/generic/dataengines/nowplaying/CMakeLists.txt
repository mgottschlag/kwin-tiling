project(plasma-dataengine-nowplaying)

set(nowplaying_engine_SRCS
    nowplayingengine.cpp
    playercontrol.cpp
    playeractionjob.cpp
    playercontainer.cpp
    playerinterface/player.cpp
    playerinterface/playerfactory.cpp
    playerinterface/dbuswatcher.cpp
    playerinterface/pollingwatcher.cpp
    playerinterface/juk.cpp
    playerinterface/mpris/mpris.cpp
    playerinterface/mpris/mprisdbustypes.cpp
    playerinterface/mpris2/mpris2.cpp
)

QT4_ADD_DBUS_INTERFACE(nowplaying_engine_SRCS playerinterface/org.kde.juk.player.xml juk_interface)

set(mpris_player_desc playerinterface/mpris/org.freedesktop.MediaPlayer.player.xml)
set_source_files_properties(${mpris_player_desc} PROPERTIES
   NO_NAMESPACE true
   INCLUDE "playerinterface/mpris/mprisdbustypes.h"
   CLASSNAME MprisPlayer
)
QT4_ADD_DBUS_INTERFACE(nowplaying_engine_SRCS ${mpris_player_desc} mprisplayer)

if(GLIB2_FOUND)
   macro_optional_find_package(Xmms)
   macro_log_feature(XMMS_FOUND "XMMS" "X MultiMedia System development libraries" "http://www.xmms.org" FALSE "" "Support for XMMS in the Now Playing data engine")
   if(XMMS_FOUND)
      include_directories(${XMMS_INCLUDE_DIRS})
      include_directories(${GLIB2_INCLUDE_DIR})
      set(nowplaying_engine_SRCS ${nowplaying_engine_SRCS} playerinterface/xmms.cpp)
      message(STATUS "Found XMMS: the nowplaying dataengine will support XMMS 1.x")
   else(XMMS_FOUND)
      message(STATUS "Could NOT find XMMS: the nowplaying dataengine will NOT support XMMS 1.x")
   endif(XMMS_FOUND)
endif(GLIB2_FOUND)

configure_file(config-nowplaying.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-nowplaying.h)

kde4_add_plugin(plasma_engine_nowplaying ${nowplaying_engine_SRCS})
target_link_libraries(plasma_engine_nowplaying ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS} ${QT_QTGUI_LIBRARY})
if(XMMS_FOUND)
   target_link_libraries(plasma_engine_nowplaying ${XMMS_LDFLAGS})
endif(XMMS_FOUND)

install(TARGETS plasma_engine_nowplaying DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-nowplaying.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES nowplaying.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

