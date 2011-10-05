project(KDEBASE_WORKSPACE)

# Used e.g. in KDE4WorkspaceConfig.cmake, Alex
set(KDE4WORKSPACE_VERSION_MAJOR 4)
set(KDE4WORKSPACE_VERSION_MINOR 6)
set(KDE4WORKSPACE_VERSION_PATCH 90)
set(KDE4WORKSPACE_VERSION ${KDE4WORKSPACE_VERSION_MAJOR}.${KDE4WORKSPACE_VERSION_MINOR}.${KDE4WORKSPACE_VERSION_PATCH} )


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

#search packages used by KDE
set(QT_MIN_VERSION "4.7.0")
set(KDE_MIN_VERSION "4.6.90")
set(SOPRANO_MIN_VERSION "2.5.63")
find_package(KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package(Strigi REQUIRED)
find_package(ZLIB REQUIRED)
find_package(KActivities REQUIRED)
find_package(DBusMenuQt 0.6.0 REQUIRED)
find_package(QImageBlitz REQUIRED)
macro_optional_find_package(OpenGL)
macro_optional_find_package(OpenGLES)

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    macro_log_feature(X11_XTest_FOUND "libXtst" "X11 Testing Resource extension library" "http://xorg.freedesktop.org" FALSE "" "useful for automated testing of X clients.")
  endif(NOT X11_XTest_FOUND)
  if(NOT X11_Xau_FOUND)
    macro_log_feature(X11_Xau_FOUND "libXau" "X11 Authorization Protocol library" "http://xorg.freedesktop.org" FALSE "" "Needed by KDM.")
  endif(NOT X11_Xau_FOUND)
  if(NOT X11_Xdmcp_FOUND)
    macro_log_feature(X11_Xdmcp_FOUND "libXdmcp" "X Display Manager Control Protocol library" "http://xorg.freedesktop.org" FALSE "" "Needed by KDM.")
  endif(NOT X11_Xdmcp_FOUND)
  # Other X11 libraries that are used unconditionally elsewhere
  # without testing if they are found. With these log messages
  # you get a nice complaint about missing required packages
  # before the CMake error list of unused variables.
  if(NOT X11_Xkbfile_FOUND)
    macro_log_feature(X11_Xkbfile_FOUND "libXkbfile" "X11 keyboard layout library" "http://xorg.freedesktop.org" TRUE "" "Needed for keyboard modules.")
  endif(NOT X11_Xkbfile_FOUND)
  if(NOT X11_Xcomposite_FOUND)
    macro_log_feature(X11_Xcomposite_FOUND "libXcomposite" "X11 composite library" "http://xorg.freedesktop.org" TRUE "" "Needed for plasma tray and notifications.")
  endif(NOT X11_Xcomposite_FOUND)
  if(NOT X11_Xdamage_FOUND)
    macro_log_feature(X11_Xdamage_FOUND "libXdamage" "X11 damaged region extension library" "http://xorg.freedesktop.org" TRUE "" "Needed for Compositing support in KWin.")
  endif(NOT X11_Xdamage_FOUND)
  if(NOT X11_Xrender_FOUND)
    macro_log_feature(X11_Xrender_FOUND "libXrender" "X Rendering Extension client library" "http://xorg.freedesktop.org" TRUE "" "Needed for XRender Compositing backend in KWin.")
  endif(NOT X11_Xrender_FOUND)
  if(NOT X11_Xfixes_FOUND)
    macro_log_feature(X11_Xfixes_FOUND "libXfixes" "X11 miscellaneous 'fixes' extension library" "http://xorg.freedesktop.org" TRUE "" "Needed for XRender Compositing backend in KWin.")
  endif(NOT X11_Xfixes_FOUND)
  if(NOT X11_Xrandr_FOUND)
    macro_log_feature(X11_Xrandr_FOUND "libXrandr" "X11 RandR extension library" "http://xorg.freedesktop.org" TRUE "" "Needed for Multi Screen Support.")
  endif(NOT X11_Xrandr_FOUND)
  if(NOT OPENGL_FOUND AND NOT OPENGLES_FOUND)
    macro_log_feature(OPENGL_FOUND "OpenGL" "OpenGL" "http://www.opengl.org/" FALSE "" "Needed to build OpenGL Compositing backend in KWin")
    macro_log_feature(OPENGLES_FOUND "OpenGL ES" "OpenGL ES 2.0" "http://www.khronos.org/opengles/" FALSE "" "Needed to build OpenGL ES 2.0 Compositing backend in KWin")
    set(OPENGL_OR_ES_FOUND FALSE)
    macro_log_feature(OPENGL_OR_ES_FOUND "OpenGL (ES)" "OpenGL (ES)" "http://mesa3d.org/" TRUE "" "Either OpenGL or OpenGL ES 2.0 are required for Compositing support in KWin.")
  endif(NOT OPENGL_FOUND AND NOT OPENGLES_FOUND)
endif(Q_WS_X11)

macro_optional_find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "glib2" "Low-level core library for data structure handling, portability wrappers, etc." "http://www.gtk.org" FALSE "2.x" "Needed to build the kxkb keyboard map control program and provide XMMS support in the Now Playing Plasma data engine")

macro_optional_find_package(Fontconfig)

macro_optional_find_package(Googlegadgets)
macro_log_feature(GOOGLEGADGETS_FOUND "GoogleGadgets" "Google Gadgets library" "http://code.google.com/p/google-gadgets-for-linux/" FALSE "0.11.0" "Needed to build plasma scriptengine for google gadgets")

#### Python support ####
macro_optional_find_package(PythonLibrary)

macro_optional_find_package(SIP)
if(SIP_FOUND AND SIP_VERSION STRLESS "040701")
  message(STATUS "The version of SIP found is too old. 4.7.1 or later is needed.")
  set(SIP_FOUND)
endif(SIP_FOUND AND SIP_VERSION STRLESS "040701")

include(SIPMacros)

macro_optional_find_package(PyQt4)
if(PYQT4_FOUND AND PYQT4_VERSION STRLESS "040400")
  message(STATUS "The version of PyQt found is too old. 4.4.0 or later is required.")
  set(PYQT4_FOUND)
endif(PYQT4_FOUND AND PYQT4_VERSION STRLESS "040400")

macro_optional_find_package(PyKDE4)
set(PythonSupport_FOUND TRUE)

#### End Python support ####

include(MacroLibrary)
include(KDE4Defaults)

macro_optional_find_package(Nepomuk)
macro_log_feature(Nepomuk_FOUND "Nepomuk" "Nepomuk Libraries" "kdelibs" FALSE "" "Required to build Nepomuk features.")
macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "www.boost.org" FALSE "" "Required to build certain Plasma DataEngines (Akonadi, RSS, Calendar)")
macro_optional_find_package(Akonadi)
macro_log_feature(Akonadi_FOUND "Akonadi" "An extensible cross-desktop storage service for PIM data" "http://pim.kde.org/akonadi" FALSE "" "Required to build certain Plasma DataEngines (Akonadi, Calendar)")
macro_optional_find_package(KdepimLibs 4.5.60)
macro_log_feature(KdepimLibs_FOUND "KDE PIM Libs" "KDE Libraries for PIM" "http://pim.kde.org/" FALSE "4.5.60" "Required to build certain Plasma DataEngines (Akonadi, Calendar)")


include(ConfigureChecks.cmake)
if(NOT WIN32)
  configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
  configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h )
  if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(EXPORT_XCURSOR_PATH "XCURSOR_PATH=${CMAKE_INSTALL_PREFIX}/share/icons:$XCURSOR_PATH\":~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons\"; export XCURSOR_PATH")
  endif(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  configure_file(startkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/startkde  @ONLY)
endif(NOT WIN32)
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h )

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KACTIVITIES_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libs)

add_definitions(-DDISABLE_NEPOMUK_LEGACY)

# libs
add_subdirectory(cmake)
add_subdirectory(libs)

# system settings (kcontrol replacement)
if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   macro_optional_add_subdirectory( systemsettings )
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

# core apps
if(NOT WIN32)
   macro_optional_add_subdirectory( kcheckpass )
   macro_optional_add_subdirectory( kwin )
   macro_optional_add_subdirectory( ksmserver )
   macro_optional_add_subdirectory( ksplash )
   macro_optional_add_subdirectory( powerdevil )
   macro_optional_add_subdirectory( qguiplatformplugin_kde )
   macro_optional_add_subdirectory( ksysguard )
endif(NOT WIN32)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   macro_optional_add_subdirectory(kcontrol)
   macro_optional_add_subdirectory(klipper)
   macro_optional_add_subdirectory(kmenuedit)
   macro_optional_add_subdirectory(krunner)
   macro_optional_add_subdirectory(solid)
   macro_optional_add_subdirectory(kstartupconfig)
   macro_optional_add_subdirectory(freespacenotifier)

   # data
   macro_optional_add_subdirectory(doc)

   if (Q_WS_X11)
      macro_optional_add_subdirectory(kinfocenter)
   endif (Q_WS_X11)

   if(NOT WIN32)
      macro_optional_add_subdirectory(kscreensaver)
      macro_optional_add_subdirectory(kcminit)
      macro_optional_add_subdirectory(khotkeys)
      macro_optional_add_subdirectory(kwrited)
      macro_optional_add_subdirectory(ksystraycmd)
      # data
      macro_optional_add_subdirectory(cursors)
   endif(NOT WIN32)

   if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
      add_subdirectory( kdm )
   else(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
      message(STATUS "Xau lib or Xdmcp lib was missing. kdm will not compile")
   endif(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

macro_optional_add_subdirectory(plasma)
macro_optional_add_subdirectory(statusnotifierwatcher)
macro_optional_add_subdirectory(kstyles)

########### install files ###############
if(NOT WIN32)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startkde DESTINATION ${BIN_INSTALL_DIR})
endif(NOT WIN32)

macro_display_feature_log()

# make the libraries installed from kdebase/workspace available to other projects
# by creating and installing a KDE4WorkspaceConfig.cmake file, which will
# be searched and found by kdelibs/cmake/modules/FindKDE4Workspace.cmake. Alex

# now create the KDE4WorkspaceConfig.cmake file, which will be loaded by 
# kdelibs/cmake/modules/FindKDE4Workspace.cmake and which has to contain all information
# about the libraries installed from kdebase/workspace/libs/ anybody would like to have. Alex

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (IS_ABSOLUTE "${in}")    # IS_ABSOLUTE is new since cmake 2.4.8
      set(${out} "${in}")
   else (IS_ABSOLUTE "${in}")
      set(${out} "\${KDE4WORKSPACE_INSTALL_DIR}/${in}")
   endif (IS_ABSOLUTE "${in}")
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

make_install_path_absolute(KDE4WORKSPACE_LIB_DIR     ${LIB_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_LIBEXEC_DIR ${LIBEXEC_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_INCLUDE_DIR ${INCLUDE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_BIN_DIR     ${BIN_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SBIN_DIR    ${SBIN_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DATA_DIR    ${DATA_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_HTML_DIR    ${HTML_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_CONFIG_DIR  ${CONFIG_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_ICON_DIR    ${ICON_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_KCFG_DIR    ${KCFG_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_LOCALE_DIR  ${LOCALE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_MIME_DIR    ${MIME_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SOUND_DIR   ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_TEMPLATES_DIR    ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_WALLPAPER_DIR    ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_KCONF_UPDATE_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_AUTOSTART_DIR    ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_XDG_APPS_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_XDG_DIRECTORY_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SYSCONF_DIR ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_MAN_DIR     ${MAN_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_INFO_DIR    ${INFO_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SERVICES_DIR ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SERVICETYPES_DIR ${SERVICETYPES_INSTALL_DIR})

set(KDE4WORKSPACE_TARGET_PREFIX KDE4Workspace__)

configure_file(KDE4WorkspaceConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake" @ONLY)

# this file will be installed too and will be used by cmake when searching for the Config.cmake file to check the version of kdepimlibs, Alex
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake 
                                     ${KDE4WORKSPACE_VERSION_MAJOR} ${KDE4WORKSPACE_VERSION_MINOR} ${KDE4WORKSPACE_VERSION_PATCH})

set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/KDE4Workspace/cmake)
# places where find_package() looks for FooConfig.cmake files:
# CMake >= 2.6.0 looks in lib/Foo*/cmake/, CMake >= 2.6.3 also looks in
# lib/cmake/Foo*/, which packagers prefer. So they can set the KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR
# option to have kdepimlibs install its Config file there. Alex
if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
   set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDE4Workspace)
endif(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR})

install(EXPORT kdeworkspaceLibraryTargets 
        NAMESPACE ${KDE4WORKSPACE_TARGET_PREFIX}
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR}
        FILE KDE4WorkspaceLibraryTargets.cmake )
