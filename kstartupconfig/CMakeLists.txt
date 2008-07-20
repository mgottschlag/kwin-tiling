########### kstartupconfig ###############

set(kstartupconfig_SRCS kstartupconfig.cpp )

kde4_add_executable(kstartupconfig4 NOGUI ${kstartupconfig_SRCS})

if(WIN32)
target_link_libraries(kstartupconfig4 ${KDE4_KDECORE_LIBS})
else(WIN32)
target_link_libraries(kstartupconfig4 kdefakes ${KDE4_KDECORE_LIBS})
endif(WIN32)

install(TARGETS kstartupconfig4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kdostartupconfig ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

kde4_add_executable(kdostartupconfig4 NOGUI ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig4 ${KDE4_KDECORE_LIBS} )

install(TARGETS kdostartupconfig4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

