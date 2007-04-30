
########### next target ###############

set(solidshell_SRCS main.cpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../control )

kde4_automoc(${solidshell_SRCS})

kde4_add_executable(solidshell ${solidshell_SRCS})

target_link_libraries(solidshell ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS solidshell DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############



