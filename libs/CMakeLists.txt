add_subdirectory(kdm)
add_subdirectory(kephal)
add_subdirectory(plasmaclock)
add_subdirectory(plasmagenericshell)
add_subdirectory(solid)

if(NOT WIN32)
 add_subdirectory(taskmanager)
 add_subdirectory(ksysguard)
endif(NOT WIN32)
add_subdirectory(kworkspace)

