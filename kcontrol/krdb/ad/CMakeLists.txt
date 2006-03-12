kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdisplay/app-defaults FILES AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad AcroRead.ad  	Editres.ad Emacs.ad GV.ad ML.ad Nedit.ad Netscape.ad RVPlayer.ad  	WPerfect.ad XCalc.ad Xawtv.ad Xdvi.ad XTerm.ad XOsview.ad XV.ad Xpdf.ad )
install_files( /bin FILES krdb)

kde4_footer()



#original Makefile.am contents follow:

#
#AM_CPPFLAGS = $(all_includes) 
#
#kdisplay_defaults_data_DATA = AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad AcroRead.ad \
#	Editres.ad Emacs.ad GV.ad ML.ad Nedit.ad Netscape.ad RVPlayer.ad \
#	WPerfect.ad XCalc.ad Xawtv.ad Xdvi.ad XTerm.ad XOsview.ad XV.ad Xpdf.ad
#
#kdisplay_defaults_datadir = $(kde_datadir)/kdisplay/app-defaults
#
#EXTRA_DIST = $(kdisplay_defaults_data_DATA)
#
#bin_SCRIPTS = krdb
#
