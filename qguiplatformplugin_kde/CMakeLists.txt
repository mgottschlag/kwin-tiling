project(qguiplatformplugin_kde)

find_package(KDE4 REQUIRED)
find_package(Qt4 REQUIRED)
set (QT_MIN_VERSION "4.6.0")
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(kde_SOURCES  qguiplatformplugin_kde.cpp)
qt4_generate_moc(qguiplatformplugin_kde.cpp qguiplatformplugin_kde.moc)

add_library (kde SHARED ${kde_SOURCES})

target_link_libraries(kde ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY})

install (TARGETS kde LIBRARY DESTINATION ${QT_PLUGINS_DIR}/gui_platform)




