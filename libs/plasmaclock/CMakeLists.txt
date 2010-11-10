find_package(KdepimLibs REQUIRED)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal
                    ${KDEPIMLIBS_INCLUDE_DIRS}
)

set(plasmaclock_LIB_SRCS
    clockapplet.cpp
    calendar.cpp
    calendartable.cpp
    wheelytoolbutton.cpp
)

kde4_add_ui_files(plasmaclock_LIB_SRCS timezonesConfig.ui calendarConfig.ui generalConfig.ui)

kde4_add_library(plasmaclock SHARED ${plasmaclock_LIB_SRCS})

target_link_libraries(plasmaclock
                      ${KDE4_PLASMA_LIBS}
                      ${KDEPIMLIBS_KHOLIDAYS_LIBRARY}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      kephal)

set_target_properties(plasmaclock PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

#clockapplet.h
install(FILES clockapplet.h calendar.h calendartable.h plasmaclock_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaclock COMPONENT Devel)
install(TARGETS plasmaclock EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
