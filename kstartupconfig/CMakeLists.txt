########### kstartupconfig ###############

set(kstartupconfig_SRCS kstartupconfig.cpp )

kde4_add_executable(kstartupconfig NOGUI ${kstartupconfig_SRCS})

target_link_libraries(kstartupconfig ${KDE4_KDECORE_LIBS})
if(NOT WIN32)
    target_link_libraries(kstartupconfig kdefakes)
endif(NOT WIN32)

install(TARGETS kstartupconfig DESTINATION ${BIN_INSTALL_DIR} )

########### kdostartupconfig ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

kde4_add_executable(kdostartupconfig NOGUI ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig ${KDE4_KDECORE_LIBS} )

install(TARGETS kdostartupconfig DESTINATION ${BIN_INSTALL_DIR} )

