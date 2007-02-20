include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

if(KDE4_BUILD_TESTS)
set (nmtest_SRCS
	listdevices.cpp
	nmobject.cpp
)

kde4_automoc(${nmtest_SRCS})
kde4_add_executable(nmtest RUN_UNINSTALLED ${nmtest_SRCS})
target_link_libraries(nmtest solid_networkmanager_static ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY})
#QT_QTTEST_LIBRARY

endif(KDE4_BUILD_TESTS)

