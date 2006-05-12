kde4_header()



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdisplay/app-defaults FILES AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad AcroRead.ad  	Editres.ad Emacs.ad GV.ad ML.ad Nedit.ad Netscape.ad RVPlayer.ad  	WPerfect.ad XCalc.ad Xawtv.ad Xdvi.ad XTerm.ad XOsview.ad XV.ad Xpdf.ad )
install(PROGRAMS krdb DESTINATION bin)

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
