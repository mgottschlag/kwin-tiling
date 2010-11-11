project(plasma-tasks)

set(tasks_SRCS
    windowtaskitem.cpp tasks.cpp taskitemlayout.cpp abstracttaskitem.cpp taskgroupitem.cpp applauncheritem.cpp)

kde4_add_ui_files(tasks_SRCS tasksConfig.ui )
kde4_add_plugin(plasma_applet_tasks ${tasks_SRCS})
target_link_libraries(plasma_applet_tasks ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} taskmanager)
if(Q_WS_X11)
    target_link_libraries(plasma_applet_tasks ${X11_LIBRARIES})
endif(Q_WS_X11)

install(TARGETS plasma_applet_tasks DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-tasks-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
