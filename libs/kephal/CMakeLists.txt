PROJECT (kephal)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

ADD_SUBDIRECTORY (xml)
ADD_SUBDIRECTORY (xrandr12)
ADD_SUBDIRECTORY (kded_kephal)
ADD_SUBDIRECTORY (libkephal)
