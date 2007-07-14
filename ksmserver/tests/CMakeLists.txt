
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDEBASE_WORKSPACE_SOURCE_DIR}/solid/control/ )

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(testsh_SRCS test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../shutdowndlg.cpp )


kde4_add_test_executable(testsh ${testsh_SRCS})

target_link_libraries(testsh plasma solidcontrol kworkspace ${KDE4_KDEUI_LIBS})

