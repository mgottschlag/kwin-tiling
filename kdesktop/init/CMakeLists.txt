
add_subdirectory( Templates ) 



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdesktop/DesktopLinks FILES Home.desktop System.desktop )
install_files( ${DATA_INSTALL_DIR}/kdesktop FILES directory.trash directory.autostart directory.templates directory.desktop )




#original Makefile.am contents follow:

#SUBDIRS = Templates
#
#kdesktop_init_data_DATA = directory.trash directory.autostart directory.templates directory.desktop
#kdesktop_init_datadir = $(kde_datadir)/kdesktop
#
#desktoplink_DATA = Home.desktop System.desktop
#desktoplinkdir = $(kde_datadir)/kdesktop/DesktopLinks
#
