
########### next target ###############

set(solidshell_SRCS main.cpp )

kde4_automoc(${solidshell_SRCS})

kde4_add_executable(solidshell ${solidshell_SRCS})

target_link_libraries(solidshell ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} )

install(TARGETS solidshell DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############



