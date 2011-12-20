if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
    add_subdirectory(animators)
endif(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory(containmentactions)
   add_subdirectory(tools)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(shells)
add_subdirectory(wallpapers)

