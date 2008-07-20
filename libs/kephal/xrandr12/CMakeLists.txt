FIND_PACKAGE (X11 REQUIRED)
FIND_PACKAGE (Qt4 REQUIRED)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. ${QT_INCLUDES} ${X11_INCLUDE_DIR})

SET (xrandr12_SRCS randr.cpp randrscreen.cpp randroutput.cpp randrcrtc.cpp
    randrmode.cpp randrdisplay.cpp)
QT4_WRAP_CPP (xrandr12_SRCS randr.h randrscreen.h randroutput.h randrcrtc.h
    randrmode.h randrdisplay.h)

ADD_LIBRARY (xrandr12 ${xrandr12_SRCS})

