


########### install files ###############

install( FILES kdelogo.png kdelogo-crystal.png shutdown.jpg default1.png default2.png default3.png root1.png  DESTINATION  ${DATA_INSTALL_DIR}/kdm/pics )




#original Makefile.am contents follow:

#
#picsdir = $(kde_datadir)/kdm/pics
#pics_DATA = kdelogo.png kdelogo-crystal.png shutdown.jpg
#
#usersdir = $(picsdir)/users
#users_DATA = default1.png default2.png default3.png root1.png
#
#
#EXTRA_DIST = $(pics_DATA) $(users_DATA)
