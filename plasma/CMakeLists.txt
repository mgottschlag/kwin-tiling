add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1204)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasma)

add_subdirectory(animators)
add_subdirectory(applets)
add_subdirectory(containments)
add_subdirectory(dataengines)
add_subdirectory(desktoptheme)
add_subdirectory(plasma)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(tools)

