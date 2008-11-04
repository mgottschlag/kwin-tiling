project(KDEBASE_WORKSPACE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(BUILDING_WORKSPACE_SEPARATELY true)
endif(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")


#search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Strigi REQUIRED)
find_package(QImageBlitz REQUIRED)
find_package(ZLIB REQUIRED)

macro_optional_find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "glib2" "Low-level core library for data structure handling, portability wrappers, etc." "http://www.gtk.org" FALSE "2.x" "Needed to build the kxkb keyboard map control program and provide XMMS support in the Now Playing Plasma data engine")

macro_optional_find_package(Fontconfig)

macro_optional_find_package(Googlegadgets)
macro_log_feature(GOOGLEGADGETS_FOUND "GoogleGadgets" "Google Gadgets library" "http://code.google.com/p/google-gadgets-for-linux/" FALSE "" "Needed to build plasma scriptengine for google gadgets")

macro_optional_find_package(QEdje)
macro_log_feature(QEdje_FOUND "QEdje" "QEdje library" "http://dev.openbossa.org/trac/qedje/" FALSE "" "Needed to build plasma scriptengine for edje files")

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
