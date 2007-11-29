set(tasks_engine_SRCS
    tasksengine.cpp
)

kde4_add_plugin(plasma_engine_tasks ${tasks_engine_SRCS})
target_link_libraries(plasma_engine_tasks ${KDE4_KDEUI_LIBS} plasma
                      taskmanager)

install(TARGETS plasma_engine_tasks DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-tasks.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

