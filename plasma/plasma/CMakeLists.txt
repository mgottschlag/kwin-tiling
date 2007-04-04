include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/workspace/plasma/lib ${CMAKE_SOURCE_DIR}/workspace/lib)

add_definitions (-DQT3_SUPPORT)

set(plasma_SRCS 
    clock.cpp
    main.cpp
    desktop.cpp
    enginemanager.cpp
    plasmaapp.cpp
    )

set(ksmserver_xml  ${CMAKE_SOURCE_DIR}/workspace/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE( plasma_SRCS ${ksmserver_xml} ksmserver_interface )


kde4_automoc(${plasma_SRCS})

kde4_add_executable(plasma-qgv ${plasma_SRCS})

target_link_libraries(plasma-qgv plasma kworkspace ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS plasma-qgv DESTINATION ${BIN_INSTALL_DIR})

