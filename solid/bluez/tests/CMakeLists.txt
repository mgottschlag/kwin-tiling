set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
set (blueztest_SRCS blueztest.cpp)


kde4_add_executable(blueztest TEST ${blueztest_SRCS})

target_link_libraries(blueztest solid_bluez_static ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY} ${QT_QTGUI_LIBRARY})
