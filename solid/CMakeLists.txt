macro_optional_find_package(NetworkManager)
macro_log_feature(NETWORKMANAGER_FOUND "NetworkManager" "NetworkManager is a daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager")

macro_optional_find_package(BlueZ)
macro_log_feature(BLUEZ_FOUND "BlueZ" "BlueZ is the linux bluetooth subsystem" "http://www.bluez.org")

add_subdirectory( solidshell )
add_subdirectory( hal )
if (NETWORKMANAGER_FOUND)
	add_subdirectory( networkmanager )
endif (NETWORKMANAGER_FOUND)
if (BLUEZ_FOUND)
	add_subdirectory( bluez )
endif (BLUEZ_FOUND)
