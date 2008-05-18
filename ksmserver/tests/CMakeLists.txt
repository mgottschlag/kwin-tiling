include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDEBASE_WORKSPACE_SOURCE_DIR}/solid/control/ ${QIMAGEBLITZ_INCLUDES} )

set(testsh_SRCS
    test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shutdowndlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../logouteffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../curtaineffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fadeeffect.cpp)

kde4_add_executable(testsh TEST ${testsh_SRCS})

target_link_libraries(testsh plasma solidcontrol kworkspace ${KDE4_KDEUI_LIBS} ${QIMAGEBLITZ_LIBRARIES})
