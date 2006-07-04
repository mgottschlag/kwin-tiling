
add_subdirectory( Templates ) 



########### install files ###############

install( FILES Home.desktop System.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdesktop/DesktopLinks )
install( FILES directory.trash directory.autostart directory.templates directory.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdesktop )




#original Makefile.am contents follow:

#SUBDIRS = Templates
#
#kdesktop_init_data_DATA = directory.trash directory.autostart directory.templates directory.desktop
#kdesktop_init_datadir = $(kde_datadir)/kdesktop
#
#desktoplink_DATA = Home.desktop System.desktop
#desktoplinkdir = $(kde_datadir)/kdesktop/DesktopLinks
#
