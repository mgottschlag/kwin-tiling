
project(ksystraycmd)



########### next target ###############

set(ksystraycmd_SRCS ksystraycmd.cpp main.cpp )

kde4_automoc(ksystraycmd ${ksystraycmd_SRCS})

kde4_add_executable(ksystraycmd ${ksystraycmd_SRCS})

target_link_libraries(ksystraycmd  ${KDE4_KDEUI_LIBS} )

install(TARGETS ksystraycmd DESTINATION ${BIN_INSTALL_DIR})

