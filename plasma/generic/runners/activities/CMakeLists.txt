include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

set(krunner_activities_SRCS
    activityrunner.cpp
)

kde4_add_plugin(krunner_activities ${krunner_activities_SRCS})
target_link_libraries(krunner_activities ${KDE4_PLASMA_LIBS} ${KACTIVITIES_LIBRARY})

install(TARGETS krunner_activities DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-activityrunner.desktop DESTINATION ${SERVICES_INSTALL_DIR})

