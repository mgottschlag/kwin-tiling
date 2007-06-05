add_subdirectory(clock)
add_subdirectory(url)

if(KDE4_SYNDICATION_LIBS)
    add_subdirectory(cia.vc)
endif(KDE4_SYNDICATION_LIBS)
