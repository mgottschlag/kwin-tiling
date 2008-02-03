project(KDEBASE_WORKSPACE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )


#search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Strigi REQUIRED)
find_package(Blitz REQUIRED)

macro_optional_find_package(USB)
macro_optional_find_package(Fontconfig)
macro_optional_find_package(OpenGL)

include (MacroLibrary)
include (KDE4Defaults)

include(ConfigureChecks.cmake)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h )
configure_file (config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h )
configure_file (startkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/startkde  @ONLY)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set( backgroundlib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/kdm/background/bgrender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/kdm/background/bgsettings.cpp
)

set( kcmbackgroundlib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/kdm/background/bgdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/kdm/background/bgadvanced.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/kdm/background/bgmonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcontrol/kdm/background/bgwallpaper.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kdm
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/kworkspace
)
# libs
add_subdirectory(cmake)
add_subdirectory(libs)

# system settings (kcontrol replacement)
add_subdirectory( systemsettings )

# core apps
add_subdirectory( kcheckpass )
add_subdirectory( kscreensaver )
add_subdirectory( solid )
add_subdirectory( ksmserver )
add_subdirectory( kcminit )
add_subdirectory( ksplash )
add_subdirectory( ksysguard )
add_subdirectory( ktip )
add_subdirectory( klipper )
add_subdirectory( kmenuedit )
add_subdirectory( krunner )
add_subdirectory( kwin )
add_subdirectory( plasma )
add_subdirectory( kstartupconfig )


add_subdirectory( khotkeys )

# apps we need to figure out what to do with
add_subdirectory( kcontrol )
add_subdirectory( ksystraycmd )

# data
add_subdirectory( doc )
add_subdirectory( wallpapers )
add_subdirectory( menu )

if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
  	add_subdirectory( kdm )
else(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
	MESSAGE(STATUS "Xau lib or Xdmcp lib was missing. kdm will not compile")
endif(X11_Xau_FOUND AND X11_Xdmcp_FOUND)

SET(krunner_DEPENDS "plasma")

########### install files ###############
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startkde DESTINATION ${BIN_INSTALL_DIR})

if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  macro_display_feature_log()
endif(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
