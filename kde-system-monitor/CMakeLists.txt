
project(kde-system-monitor)
message(STATUS "Building kde-system-monitor")
add_subdirectory( CContLib )
add_subdirectory( gui )
add_subdirectory( pics )
add_subdirectory( example )
add_subdirectory( kde-system-monitor-daemon )



########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




#original Makefile.am contents follow:

#if include_ksysguardd
#KSYSGUARDD_DIR = ksysguardd
#endif
#
## claim, which subdirectories you want to install
#SUBDIRS = CContLib $(KSYSGUARDD_DIR) gui pics example
#
#KDE_ICON = kde-system-monitor
#
