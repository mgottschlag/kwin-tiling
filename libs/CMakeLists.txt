add_subdirectory(kephal)
add_subdirectory(kworkspace)
add_subdirectory(oxygen)
add_subdirectory(plasmaclock)
add_subdirectory(solid)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory(kdm)
   add_subdirectory(plasmagenericshell)
   add_subdirectory(taskmanager)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

if(NOT WIN32)
 add_subdirectory(ksysguard)
endif(NOT WIN32)

