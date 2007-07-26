find_package(KDE4 REQUIRED)
find_package(Qt4 REQUIRED)

include(KDE4Defaults)
include(${QT_USE_FILE})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(filebrowser_engine_SRCS
    filebrowserengine.cpp
)

kde4_add_plugin(plasma_engine_filebrowser ${filebrowser_engine_SRCS})

target_link_libraries(plasma_engine_filebrowser ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_filebrowser DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-filebrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

