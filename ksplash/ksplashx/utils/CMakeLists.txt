
add_definitions (-DQT3_SUPPORT)

########### next target ###############

set( prepareanim_SRCS
    prepareanim.cpp )

kde4_add_executable( prepareanim ${prepareanim_SRCS})
target_link_libraries( prepareanim ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

########### next target ###############

set( shade_SRCS
    shade.cpp )

kde4_add_executable( shade ${shade_SRCS})
target_link_libraries( shade ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
