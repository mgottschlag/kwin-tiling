set(plasmoidviewer_SRCS
    fullview.cpp
    main.cpp
)

kde4_add_executable(plasmoidviewer ${plasmoidviewer_SRCS})

target_link_libraries(plasmoidviewer plasma)

install(TARGETS plasmoidviewer ${INSTALL_TARGETS_DEFAULT_ARGS})

