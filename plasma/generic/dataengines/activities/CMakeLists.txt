project(activityengine)
include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(activity_engine_SRCS
        activityengine.cpp
        activityservice.cpp
        activityjob.cpp)

kde4_add_plugin(plasma_engine_activities ${activity_engine_SRCS})
target_link_libraries(plasma_engine_activities
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
                        kworkspace)

install(TARGETS plasma_engine_activities
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-activities.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES activities.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
