include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/workspace/plasma/lib ${CMAKE_SOURCE_DIR}/workspace/lib)

add_definitions (-DQT3_SUPPORT)

set(plasma_SRCS
    clock.cpp
    desktop.cpp
    enginemanager.cpp
    main.cpp
    plasmaapp.cpp
    rootwidget.cpp
)

set(ksmserver_xml ${CMAKE_SOURCE_DIR}/workspace/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_automoc(${plasma_SRCS})
kde4_add_executable(plasma ${plasma_SRCS})

target_link_libraries(plasma plasmoid kworkspace ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma DESTINATION ${BIN_INSTALL_DIR})

