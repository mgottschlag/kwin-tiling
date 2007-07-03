include_directories(${CMAKE_SOURCE_DIR}/workspace/libs)

set(tasks_engine_SRCS
    tasksengine.cpp
    taskmanager.cpp
)

kde4_automoc(${tasks_engine_SRCS})
kde4_add_plugin(plasma_engine_tasks ${tasks_engine_SRCS})
target_link_libraries(plasma_engine_tasks ${KDE4_KDECORE_LIBS} plasma)

install(TARGETS plasma_engine_tasks DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-tasks.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

