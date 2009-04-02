set(ksystraycmd_SRCS ksystraycmd.cpp main.cpp )


kde4_add_executable(ksystraycmd ${ksystraycmd_SRCS})

target_link_libraries(ksystraycmd ${KDE4_KDEUI_LIBS})
if (Q_WS_X11)
  target_link_libraries(ksystraycmd ${X11_X11_LIB})
endif (Q_WS_X11)

install(TARGETS ksystraycmd ${INSTALL_TARGETS_DEFAULT_ARGS})
