kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdesktop/patterns FILES night-rock.desktop night-rock.jpg stonewall2.png      stonewall2.desktop fish.desktop fish.png flowers.desktop      flowers.png pavement.desktop pavement.png rattan.desktop rattan.png      triangles.desktop triangles.png )

kde4_footer()



#original Makefile.am contents follow:

#patdir=$(kde_datadir)/kdesktop/patterns
#pat_DATA = night-rock.desktop night-rock.jpg stonewall2.png \
#    stonewall2.desktop fish.desktop fish.png flowers.desktop \
#    flowers.png pavement.desktop pavement.png rattan.desktop rattan.png \
#    triangles.desktop triangles.png
