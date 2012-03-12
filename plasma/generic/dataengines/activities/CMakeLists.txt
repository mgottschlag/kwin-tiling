project(activityengine)

set(activity_engine_SRCS
        ActivityData.cpp
        activityengine.cpp
        activityservice.cpp
        activityjob.cpp)

set_source_files_properties(org.kde.ActivityManager.ActivityRanking.xml PROPERTIES INCLUDE "ActivityData.h")
qt4_add_dbus_interface(
   activity_engine_SRCS org.kde.ActivityManager.ActivityRanking.xml
   ActivityRankingInterface
   )

kde4_add_plugin(plasma_engine_activities ${activity_engine_SRCS})
target_link_libraries(plasma_engine_activities
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${KACTIVITIES_LIBRARY})

install(TARGETS plasma_engine_activities
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-activities.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES activities.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
