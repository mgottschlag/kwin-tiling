project (PolicyKit-kde)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (FindPkgConfig)
include (FindPolkitQt)

pkg_check_modules (POLKIT polkit>=0.8)
pkg_check_modules (POLKITDBUS polkit-dbus>=0.8)
pkg_check_modules (POLKITGRANT polkit-grant>=0.8)
FIND_PACKAGE (PolkitQt)

MESSAGE(STATUS " ${POLKIT_INCLUDE_DIRS} ===========")
if (POLKIT_FOUND AND POLKITDBUS_FOUND AND POLKITGRANT_FOUND AND POLKITQT_FOUND)
   include_directories(
      ${KDE4_INCLUDES}
      ${POLKIT_INCLUDE_DIRS}
      ${POLKITDBUS_INCLUDE_DIRS}
      ${POLKITQT_INCLUDE_DIR}
    )

  add_subdirectory(manager)
  add_subdirectory(services)
  add_subdirectory(authorization)
endif (POLKIT_FOUND AND POLKITDBUS_FOUND AND POLKITGRANT_FOUND AND POLKITQT_FOUND)
