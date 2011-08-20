include(MacroEnsureVersion)
macro_optional_find_package(NetworkManager)
    
add_subdirectory(kcm)
add_subdirectory(solid-actions-kcm)

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  add_subdirectory(solidshell)
ENDIF("${CMAKE_SYSTEM}" MATCHES "Linux")

set(HAVE_NETWORKMANAGER FALSE)
if(NETWORKMANAGER_FOUND)
  if(${NETWORKMANAGER_VERSION} VERSION_GREATER "0.6.9999" AND
     ${NETWORKMANAGER_VERSION} VERSION_LESS "0.8.1001")
    message(STATUS "Will build networkmanager-0.7")
    set(HAVE_NETWORKMANAGER TRUE)
    add_subdirectory(networkmanager-0.7)
  else()
    message(STATUS "Sorry, networkmanager-0.7 will not be built since your NetworkManager version (${NETWORKMANAGER_VERSION}) is not currently supported")
  endif()
endif (NETWORKMANAGER_FOUND)
macro_log_feature(HAVE_NETWORKMANAGER "NetworkManager" "A daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager" FALSE "0.7.0" "Needed for building Network Management support in Solid. Max version supported is 0.8.10")

set(HAVE_MODEMMANAGER FALSE)
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  message(STATUS "Will build modemmanager-0.4")
  set(HAVE_MODEMMANAGER TRUE)
  add_subdirectory(modemmanager-0.4)
endif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

if(NOT WIN32)
  macro_optional_add_subdirectory(wicd)
endif(NOT WIN32)
