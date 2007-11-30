set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
set (nmtest_SRCS
	listdevices.cpp
	nmobject.cpp
)

set (nmwatcher_SRCS
   monitorstatus.cpp
)

kde4_add_executable(nmtest TEST ${nmtest_SRCS})
kde4_add_executable(nmwatcher TEST ${nmwatcher_SRCS})
target_link_libraries(nmwatcher solid_networkmanager_static solidcontrol ${NM-UTIL_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY} ${QT_QTGUI_LIBRARY})
target_link_libraries(nmtest solid_networkmanager_static solidcontrol ${NM-UTIL_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY} ${QT_QTGUI_LIBRARY})
#QT_QTTEST_LIBRARY

