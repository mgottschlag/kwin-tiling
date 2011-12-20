add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1204)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory(desktop)
   add_subdirectory(netbook)
   add_subdirectory(screensaver)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory(generic)

