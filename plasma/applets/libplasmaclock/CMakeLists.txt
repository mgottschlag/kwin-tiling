project(libplasmaclock)

set(plasmaclock_LIB_SRCS
    clocknumber.cpp
    #clockapplet.cpp
)

kde4_add_ui_files(plasmaclock_LIB_SRCS calendar.ui )

kde4_add_library(plasmaclock SHARED ${plasmaclock_LIB_SRCS})

target_link_libraries(plasmaclock plasma ${KDE4_KDEUI_LIBS})

set_target_properties(plasmaclock PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

#clockapplet.h
install(FILES clocknumber.h plasmaclock_export.h
${CMAKE_CURRENT_BINARY_DIR}/ui_calendar.h DESTINATION ${INCLUDE_INSTALL_DIR}/)
install(TARGETS plasmaclock ${INSTALL_TARGETS_DEFAULT_ARGS} )
