kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdm/themes/circles FILES GdmGreeterTheme.desktop  	circles.xml  	background.svg  	flower.png   	help.png  	options.png  	screenshot.png )

kde4_footer()



#original Makefile.am contents follow:

#circlesdir = $(kde_datadir)/kdm/themes/circles
#circles_DATA = \
#	GdmGreeterTheme.desktop \
#	circles.xml \
#	background.svg \
#	flower.png  \
#	help.png \
#	options.png \
#	screenshot.png
#
#EXTRA_DIST = $(circles_DATA)
