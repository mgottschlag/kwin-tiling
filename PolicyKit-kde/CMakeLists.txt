pkg_check_modules (POLKIT polkit>=0.8)
pkg_check_modules (POLKITDBUS polkit-dbus>=0.8)
pkg_check_modules (POLKITGRANT polkit-grant>=0.8)

MESSAGE(STATUS " ${POLKIT_INCLUDE_DIRS} ===========")
if (POLKIT_FOUND AND POLKITDBUS_FOUND AND POLKITGRANT_FOUND AND POLKITQT_FOUND)
   include_directories(
      ${POLKIT_INCLUDE_DIRS}
      ${POLKITDBUS_INCLUDE_DIRS}
      ${POLKITQT_INCLUDE_DIR}
    )

  add_subdirectory(manager)
  add_subdirectory(services)
  add_subdirectory(authorization)
endif (POLKIT_FOUND AND POLKITDBUS_FOUND AND POLKITGRANT_FOUND AND POLKITQT_FOUND)
