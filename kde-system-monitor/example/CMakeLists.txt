


########### install files ###############

install_files( ${SYSCONF_INSTALL_DIR} FILES kde-system-monitor-daemonrc )
install_files( ${DATA_INSTALL_DIR}/kde-system-monitor FILES eventsrc )




#original Makefile.am contents follow:

#kde-system-monitor_example_datadir = $(kde_datadir)/kde-system-monitor
#kde-system-monitor_example_data_DATA = eventsrc
#
#etcdir = $(sysconfdir)
#etc_DATA = kde-system-monitor-daemonrc
