include_directories( ${CMAKE_SOURCE_DIR}/workspace/plasma/lib   )


########### next target ###############

set(pdesktop_SRCS desktop.cpp main.cpp )

kde4_automoc(${pdesktop_SRCS})

kde4_add_executable(pdesktop ${pdesktop_SRCS})

target_link_libraries(pdesktop  plasma ${KDE4_KIO_LIBS} )

install(TARGETS pdesktop DESTINATION ${BIN_INSTALL_DIR})


