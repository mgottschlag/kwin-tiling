include_directories(${CMAKE_SOURCE_DIR}/workspace/libs)

set(tasks_engine_SRCS
    tasksengine.cpp
    taskmanager.cpp
)

kde4_automoc(${tasks_engine_SRCS})
kde4_add_plugin(plasma_engine_tasks ${tasks_engine_SRCS})
target_link_libraries(plasma_engine_tasks ${KDE4_KDECORE_LIBS} plasma )

# pull in the various X libraries
if (X11_Xfixes_FOUND)
  target_link_libraries(plasma_engine_tasks ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (X11_Xrender_FOUND)
  target_link_libraries(plasma_engine_tasks ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)
if (X11_Xcomposite_FOUND)
  target_link_libraries(plasma_engine_tasks ${X11_Xcomposite_LIB})
endif (X11_Xcomposite_FOUND)

install(TARGETS plasma_engine_tasks DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-tasks.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

