
if(KDE4_BUILD_TESTS)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
set (nmtest_SRCS
	listdevices.cpp
	nmobject.cpp
)

kde4_automoc(${nmtest_SRCS})
kde4_add_executable(nmtest RUN_UNINSTALLED ${nmtest_SRCS})
target_link_libraries(nmtest solid_networkmanager_static ${KDE4_SOLID_LIBS} ${NM-UTIL_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY} ${QT_QTGUI_LIBRARY})
#QT_QTTEST_LIBRARY

endif(KDE4_BUILD_TESTS)

