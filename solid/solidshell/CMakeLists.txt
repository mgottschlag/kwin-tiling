kde4_header()

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(solidshell_SRCS main.cpp )

kde4_automoc(${solidshell_SRCS})

kde4_add_executable(solidshell ${solidshell_SRCS})

target_link_libraries(solidshell ${KDE4_KDECORE_LIBS} solid )

install_targets(/bin solidshell )


########### install files ###############


kde4_footer()
