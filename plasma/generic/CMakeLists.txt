if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
    add_subdirectory(animators)
endif(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

add_subdirectory(applets)
add_subdirectory(containmentactions)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(shells)
add_subdirectory(tools)
add_subdirectory(wallpapers)

