project(plasma-tasks)

set(tasks_SRCS
    abstracttaskitem.cpp startuptaskitem.cpp taskgroupitem.cpp
    windowtaskitem.cpp tasks.cpp)

kde4_add_plugin(plasma_applet_tasks ${tasks_SRCS})
target_link_libraries(plasma_applet_tasks ${KDE4_KDEUI_LIBS} plasma taskmanager)

install(TARGETS plasma_applet_tasks DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-tasks-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
