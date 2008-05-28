set(plasmapkg_SRCS
    main.cpp
)

kde4_add_executable(plasmapkg ${plasmapkg_SRCS})

target_link_libraries(plasmapkg plasma)

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})

