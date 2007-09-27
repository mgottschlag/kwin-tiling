set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDEBASE_WORKSPACE_SOURCE_DIR}/solid/control/ ${BLITZ_INCLUDES} )

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(testsh_SRCS test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../shutdowndlg.cpp )


kde4_add_executable(testsh TEST ${testsh_SRCS})

target_link_libraries(testsh plasma solidcontrol kworkspace ${KDE4_KDEUI_LIBS} ${BLITZ_LIBRARIES})

