project(KDEBASE_WORKSPACE)

# Used e.g. in KDE4WorkspaceConfig.cmake, Alex
set(KDE4WORKSPACE_VERSION_MAJOR 4)
set(KDE4WORKSPACE_VERSION_MINOR 3)
set(KDE4WORKSPACE_VERSION_PATCH 0)
set(KDE4WORKSPACE_VERSION ${KDE4WORKSPACE_VERSION_MAJOR}.${KDE4WORKSPACE_VERSION_MINOR}.${KDE4WORKSPACE_VERSION_PATCH} )


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(BUILDING_WORKSPACE_SEPARATELY true)
endif(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")


#search packages used by KDE
set(KDE_MIN_VERSION "4.3.0") # for the < 4.2 macro
find_package(KDE4 4.3.0 REQUIRED)
find_package(Strigi REQUIRED)
find_package(QImageBlitz REQUIRED)
find_package(ZLIB REQUIRED)

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    macro_log_feature(X11_XTest_FOUND "libXtst" "X11 Testing Resource extension library" "http://xorg.freedesktop.org" FALSE "" "useful for automated testing of X clients.")
  endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

macro_optional_find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "glib2" "Low-level core library for data structure handling, portability wrappers, etc." "http://www.gtk.org" FALSE "2.x" "Needed to build the kxkb keyboard map control program and provide XMMS support in the Now Playing Plasma data engine")

macro_optional_find_package(Fontconfig)

macro_optional_find_package(Googlegadgets)
macro_log_feature(GOOGLEGADGETS_FOUND "GoogleGadgets" "Google Gadgets library" "http://code.google.com/p/google-gadgets-for-linux/" FALSE "" "Needed to build plasma scriptengine for google gadgets")

macro_optional_find_package(QEdje)
macro_log_feature(QEDJE_FOUND "QEdje" "QEdje library" "http://code.openbossa.org/projects/qedje/pages/Home" FALSE "" "Needed to build plasma scriptengine for edje files")

macro_optional_find_package(OpenGL)

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
set(PythonSupport_FOUND FALSE)
if(PYTHONLIBRARY_FOUND AND SIP_FOUND AND PYQT4_FOUND AND PYKDE4_FOUND)
  set(PythonSupport_FOUND TRUE)
endif(PYTHONLIBRARY_FOUND AND SIP_FOUND AND PYQT4_FOUND AND PYKDE4_FOUND)

macro_log_feature(PythonSupport_FOUND "Python" "KDE Python support" "http://techbase.kde.org/Development/Languages/Python" FALSE "" "Needed to build plasma scriptengine for Python")
#### End Python support ####

include(MacroLibrary)
include(KDE4Defaults)

macro_optional_find_package(Nepomuk)
macro_log_feature(Nepomuk_FOUND "Nepomuk" "Nepomuk Libraries" "kdelibs" FALSE "" "Required to build Nepomuk features.")

macro_optional_find_package(PolkitQt)
macro_log_feature(POLKITQT_FOUND "Polkit-Qt" "Qt Wrapper around PolicyKit" "kdesupport" FALSE "" "Required to build PolicyKit-KDE")

include(ConfigureChecks.cmake)
if(NOT WIN32)
  configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
  configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h )
  if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(EXPORT_XCURSOR_PATH "XCURSOR_PATH=${CMAKE_INSTALL_PREFIX}/share/icons:$XCURSOR_PATH\":~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons\"; export XCURSOR_PATH")
  endif(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  configure_file(startkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/startkde  @ONLY)
  configure_file(safestartkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/safestartkde  @ONLY)
endif(NOT WIN32)
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h )

macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
macro_log_feature(HAVE_XSCREENSAVER "libxss" "XScreenSaver Library" "http://www.x.org/" FALSE "" "Needed to enable fallback query methods in PowerDevil power manager")
macro_log_feature(HAVE_XSYNC "libxext" "XSync Extension" "http://www.x.org/" FALSE "" "Needed to enable high efficency query in PowerDevil power manager. Strongly recommended.")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

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
macro_optional_add_subdirectory( systemsettings )

# core apps
if(NOT WIN32)
  macro_optional_add_subdirectory( kcheckpass )
  macro_optional_add_subdirectory( kscreensaver )
  macro_optional_add_subdirectory( kwin )
  macro_optional_add_subdirectory( ksmserver )
  macro_optional_add_subdirectory( kcminit )
  macro_optional_add_subdirectory( ksplash )
  macro_optional_add_subdirectory( khotkeys )
  macro_optional_add_subdirectory( klipper )
  macro_optional_add_subdirectory( ksysguard )
  macro_optional_add_subdirectory( powerdevil )
  macro_optional_add_subdirectory( kwrited )
  if(POLKITQT_FOUND)
    macro_optional_add_subdirectory( PolicyKit-kde )
  endif(POLKITQT_FOUND)
endif(NOT WIN32)
macro_optional_add_subdirectory( krunner )
macro_optional_add_subdirectory( solid )
macro_optional_add_subdirectory( kmenuedit )
macro_optional_add_subdirectory( plasma )
macro_optional_add_subdirectory( kstartupconfig )


# apps we need to figure out what to do with
macro_optional_add_subdirectory( kcontrol )
if(NOT WIN32)
  macro_optional_add_subdirectory( ksystraycmd )
endif(NOT WIN32)
# data
if(NOT WIN32)
  macro_optional_add_subdirectory( cursors )
endif(NOT WIN32)
macro_optional_add_subdirectory( doc )
macro_optional_add_subdirectory( wallpapers )

if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
  add_subdirectory( kdm )
else(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
  message(STATUS "Xau lib or Xdmcp lib was missing. kdm will not compile")
endif(X11_Xau_FOUND AND X11_Xdmcp_FOUND)


########### install files ###############
if(NOT WIN32)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startkde DESTINATION ${BIN_INSTALL_DIR})
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/safestartkde DESTINATION ${BIN_INSTALL_DIR})
endif(NOT WIN32)

if(BUILDING_WORKSPACE_SEPARATELY)
  macro_display_feature_log()
endif(BUILDING_WORKSPACE_SEPARATELY)

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

set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/KDE4Workspace-${KDE4WORKSPACE_VERSION}/cmake)
# places where find_package() looks for FooConfig.cmake files:
# CMake >= 2.6.0 looks in lib/Foo*/cmake/, CMake >= 2.6.3 also looks in
# lib/cmake/Foo*/, which packagers prefer. So they can set the KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR
# option to have kdepimlibs install its Config file there. Alex
if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
   set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDE4Workspace-${KDE4WORKSPACE_VERSION} )
endif(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR})

install(EXPORT kdeworkspaceLibraryTargets 
        NAMESPACE ${KDE4WORKSPACE_TARGET_PREFIX}
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR}
        FILE KDE4WorkspaceLibraryTargets.cmake )
