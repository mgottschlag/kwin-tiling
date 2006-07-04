


########### install files ###############

install( FILES kde32b1.khotkeys konqueror_gestures_kde321.khotkeys  DESTINATION  ${DATA_INSTALL_DIR}/khotkeys )
install( FILES khotkeys_32b1_update.upd konqueror_gestures_kde321_update.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )




#original Makefile.am contents follow:

#khotkeys_data_DATA = kde32b1.khotkeys konqueror_gestures_kde321.khotkeys
#khotkeys_datadir = $(kde_datadir)/khotkeys
#
#khotkeys_update_DATA = khotkeys_32b1_update.upd konqueror_gestures_kde321_update.upd
#khotkeys_updatedir = $(kde_datadir)/kconf_update
#
#EXTRA_DIST = $(khotkeys_data_DATA) $(khotkeys_update_DATA)
