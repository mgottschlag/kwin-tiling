
if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
set (blueztest_SRCS blueztest.cpp)


kde4_automoc(${blueztest_SRCS})
kde4_add_test(blueztest RUN_UNINSTALLED ${blueztest_SRCS})

target_link_libraries(blueztest solid_bluez_static ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY} ${QT_QTGUI_LIBRARY})

#add_test(blueztest ${EXECUTABLE_OUTPUT_PATH}/blueztest)

endif(KDE4_BUILD_TESTS)
