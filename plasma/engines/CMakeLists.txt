find_package(KdepimLibs)

add_subdirectory("time")

if(KDE4_SYNDICATION_LIBRARY)
    add_subdirectory("cia.vc")
endif(KDE4_SYNDICATION_LIBRARY)
