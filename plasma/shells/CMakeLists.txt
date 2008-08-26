add_subdirectory(desktop)
add_subdirectory(plasmoidviewer)
add_subdirectory(screensaver)

if(NOT WIN32)
   add_subdirectory(mid)
endif(NOT WIN32)

