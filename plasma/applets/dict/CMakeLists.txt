project(plasma-dict)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/workspace/plasma/lib ${CMAKE_SOURCE_DIR}/workspace/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${KDE4_INCLUDES} ${QT4_INCLUDES})

set(dict_SRCS
    dict.cpp)

kde4_automoc(${dict_SRCS})

#kde4_add_ui_files(dict_SRCS config.ui )

kde4_add_plugin(plasma_applet_dict ${dict_SRCS})
target_link_libraries(plasma_applet_dict plasma ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS})
set_target_properties(plasma_applet_dict PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS plasma_applet_dict DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dict-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
