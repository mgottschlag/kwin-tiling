include_directories( ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kxkb )

link_directories(lib)

########### next target ###############

set(kcm_keyboard_test_SRCS kcm_keyboard_test.cpp )

kde4_automoc(${kcm_keyboard_test_SRCS})

kde4_add_executable(kcm_keyboard_test ${kcm_keyboard_test_SRCS})

target_link_libraries(kcm_keyboard_test ${KDE4_KDECORE_LIBS})

SET(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${CMAKE_BINARY_DIR}/lib/kcm_keyboard.so )
