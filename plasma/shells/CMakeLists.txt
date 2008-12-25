add_subdirectory(common)
add_subdirectory(desktop)
add_subdirectory(plasmoidviewer)

if(NOT WIN32)
   add_subdirectory(screensaver)
endif(NOT WIN32)

