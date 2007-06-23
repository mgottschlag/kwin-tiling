add_subdirectory("time")
add_subdirectory("soliddevice")

if(KDE4_SYNDICATION_LIBS)
    add_subdirectory("cia.vc")
endif(KDE4_SYNDICATION_LIBS)
