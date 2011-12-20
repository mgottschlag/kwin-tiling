macro_bool_to_01(KDEPIMLIBS_FOUND HAVE_KDEPIMLIBS)
configure_file(config-calendartable.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-calendartable.h)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

set(plasmaclock_LIB_SRCS
    clockapplet.cpp
    calendar.cpp
    calendartable.cpp
    wheelytoolbutton.cpp
)

if(KDEPIMLIBS_FOUND)
    kde4_add_ui_files(plasmaclock_LIB_SRCS timezonesConfig.ui generalConfig.ui calendarHolidaysConfig.ui)
else(KDEPIMLIBS_FOUND)
    kde4_add_ui_files(plasmaclock_LIB_SRCS timezonesConfig.ui generalConfig.ui calendarConfig.ui)
endif(KDEPIMLIBS_FOUND)

kde4_add_library(plasmaclock SHARED ${plasmaclock_LIB_SRCS})

target_link_libraries(plasmaclock
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      kephal)

if(KDEPIMLIBS_FOUND)
    include_directories(${KDEPIMLIBS_INCLUDE_DIRS})
    target_link_libraries(plasmaclock ${KDEPIMLIBS_KHOLIDAYS_LIBRARY})
endif(KDEPIMLIBS_FOUND)

set_target_properties(plasmaclock PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

#clockapplet.h
install(FILES clockapplet.h calendar.h calendartable.h plasmaclock_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaclock COMPONENT Devel)
install(TARGETS plasmaclock EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
