add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1204)

########### next target ###############

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(taskmanager_LIB_SRCS
   abstractgroupableitem.cpp
   abstractgroupingstrategy.cpp
   abstractsortingstrategy.cpp
   groupmanager.cpp
   launcheritem.cpp
   startup.cpp
   strategies/alphasortingstrategy.cpp
   strategies/desktopsortingstrategy.cpp
   strategies/programgroupingstrategy.cpp
   strategies/manualgroupingstrategy.cpp
   strategies/manualsortingstrategy.cpp
   strategies/kustodiangroupingstrategy.cpp
   task.cpp
   taskactions.cpp
   taskgroup.cpp
   taskitem.cpp
   taskmanager.cpp
   tasksmodel.cpp
   launcherconfig.cpp
   launcherproperties.cpp
   )

kde4_add_ui_files(taskmanager_LIB_SRCS launcherconfig.ui launcherproperties.ui)

if(WIN32)
    set(taskmanager_LIB_SRCS ${taskmanager_LIB_SRCS} task_win.cpp)
else(WIN32)
    set(taskmanager_LIB_SRCS ${taskmanager_LIB_SRCS} task_x11.cpp)
endif(WIN32)

kde4_add_library(taskmanager SHARED ${taskmanager_LIB_SRCS})

target_link_libraries(taskmanager processcore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES})
if (X11_Xfixes_FOUND)
  target_link_libraries(taskmanager ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (X11_Xrender_FOUND)
  target_link_libraries(taskmanager ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)
if (X11_Xcomposite_FOUND)
  target_link_libraries(taskmanager ${X11_Xcomposite_LIB})
endif (X11_Xcomposite_FOUND)

if (NOT KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)
    target_link_libraries(taskmanager kephal)
 endif (NOT KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

target_link_libraries(taskmanager kworkspace)

set_target_properties(taskmanager PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS taskmanager EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
   abstractgroupableitem.h
   abstractgroupingstrategy.h
   abstractsortingstrategy.h
   groupmanager.h
   launcheritem.h
   startup.h
   task.h
   taskactions.h
   taskgroup.h
   taskitem.h
   taskmanager.h
   taskmanager_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/taskmanager COMPONENT Devel
   )


#set(tasksmodel_SRCS 
#   modeltest/dynamictreemodel.cpp
#   modeltest/modeltest.cpp
#   modeltest/main.cpp
#   )

#kde4_add_executable(tasksmodeltest ${tasksmodel_SRCS})
#target_link_libraries(tasksmodeltest ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES} taskmanager kephal kworkspace)


