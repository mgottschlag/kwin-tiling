


########### install files ###############

install( FILES GdmGreeterTheme.desktop  	circles.xml  	background.svg  	flower.png   	help.png  	options.png  	screenshot.png  DESTINATION  ${DATA_INSTALL_DIR}/kdm/themes/circles )




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
