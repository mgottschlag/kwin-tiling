include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${QIMAGEBLITZ_INCLUDES} )

set(testsh_SRCS
    test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shutdowndlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../logouteffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../curtaineffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fadeeffect.cpp)

kde4_add_executable(testsh TEST ${testsh_SRCS})

target_link_libraries(testsh ${KDE4_PLASMA_LIBS} solidcontrol kworkspace ${KDE4_KDEUI_LIBS}
                             ${KDE4_SOLID_LIBRARY} ${QIMAGEBLITZ_LIBRARIES} ${X11_X11_LIB})
