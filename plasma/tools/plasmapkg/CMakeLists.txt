set(plasmapkg_SRCS
    main.cpp
)

kde4_add_executable(plasmapkg ${plasmapkg_SRCS})

target_link_libraries(plasmapkg plasma)

install(TARGETS plasmapkg DESTINATION ${BIN_INSTALL_DIR})

