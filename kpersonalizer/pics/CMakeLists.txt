kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kpersonalizer/pics FILES step1.png step2.png step3.png step4.png step5.png )

kde4_footer()



#original Makefile.am contents follow:

#kpersonalizer_pics_data_DATA = step1.png step2.png step3.png step4.png step5.png
#kpersonalizer_pics_datadir = $(kde_datadir)/kpersonalizer/pics
#
