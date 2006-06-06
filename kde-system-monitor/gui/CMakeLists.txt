
add_subdirectory( ksgrd ) 
add_subdirectory( SensorDisplayLib ) 

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker/ ${CMAKE_SOURCE_DIR}/kicker/libkicker/  ${CMAKE_CURRENT_SOURCE_DIR}/ksgrd/ ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ )


########### next target ###############

set(libsensordisplays_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorDisplay.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/BarGraph.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DancingBars.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DancingBarsSettings.cc 
	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DummyDisplay.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/FancyPlotter.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/FancyPlotterSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListView.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/LogFile.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeter.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeterSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/Process.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessController.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessModel.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessFilter.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ReniceDlg.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLogger.cc
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerDlg.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListViewSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SignalPlotter.cc 
	)
kde4_add_ui3_files( libsensordisplays_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListViewSettingsWidget.ui 
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/LogFileSettings.ui 
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeterSettingsWidget.ui 
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerDlgWidget.ui 
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerSettingsWidget.ui )

kde4_add_ui_files( libsensordisplays_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessWidgetUI.ui )


set(kde-system-monitor_SRCS ${libsensordisplays_SRCS}
   SensorBrowser.cc 
   WorkSheet.cc
   SensorFrame.cc
   WorkSheetSettings.cc 
   Workspace.cc 
   kde-system-monitor.cc )

kde4_automoc(${kde-system-monitor_SRCS})


kde4_add_executable(kde-system-monitor ${kde-system-monitor_SRCS})

target_link_libraries(kde-system-monitor  ${KDE4_KIO_LIBS} ksgrd  )

install(TARGETS kde-system-monitor DESTINATION bin)


########### next target ###############

set(kpm_SRCS kpm.c )

kde4_automoc(${kpm_SRCS})

kde4_add_executable(kpm ${kpm_SRCS})

target_link_libraries(kpm  ${KDE4_KDECORE_LIBS} )

install(TARGETS kpm DESTINATION bin)


########### next target ###############

set(sysguard_panelapplet_PART_SRCS kde-system-monitor-applet.cc kde-system-monitor-applet-settings.cc ${libsensordisplays_SRCS} )

kde4_automoc(${sysguard_panelapplet_PART_SRCS})

kde4_add_plugin(sysguard_panelapplet ${sysguard_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} sysguard_panelapplet )

target_link_libraries(sysguard_panelapplet  ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ksgrd  kickermain  )

install_targets(${PLUGIN_INSTALL_DIR} sysguard_panelapplet )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kde-system-monitor.desktop )
install_files( ${DATA_INSTALL_DIR}/kde-system-monitor FILES ProcessTable.sgrd SystemLoad.sgrd kde-system-monitor-applet.xml )
install_files( ${MIME_INSTALL_DIR}/application FILES x-kde-system-monitor.desktop )
install_files( ${DATA_INSTALL_DIR}/kde-system-monitor FILES kde-system-monitorui.rc )
install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES kde-system-monitorapplet.desktop )




#original Makefile.am contents follow:

#
#kdemimedir = $(kde_mimedir)/application
#kdemime_DATA = x-kde-system-monitor.desktop
#
#rcdir = $(kde_datadir)/kde-system-monitor
#rc_DATA = kde-system-monitorui.rc
#
#xdg_apps_DATA = kde-system-monitor.desktop
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = kde-system-monitorapplet.desktop
#
## claim, which subdirectories you want to install
#SUBDIRS = ksgrd SensorDisplayLib
#
## set the include path for X, qt and KDE
#INCLUDES = -I$(srcdir)/ksgrd -I$(srcdir)/SensorDisplayLib -ISensorDisplayLib -I$(top_srcdir)/kicker/libkicker/ $(all_includes)
# 
######## This part is very kde-system-monitor specific
## you can add here more. This one gets installed 
#bin_PROGRAMS = kde-system-monitor kpm
#
## Which sources should be compiled for kde-system-monitor.
#kde-system-monitor_SOURCES = \
#	SensorBrowser.cc \
#	WorkSheet.cc \
#	WorkSheetSettings.cc \
#	Workspace.cc \
#	kde-system-monitor.cc kde-system-monitor.skel
#
#kde-system-monitor_LDADD = \
#	ksgrd/libksgrd.la \
#	SensorDisplayLib/libsensordisplays.la \
#	$(LIB_KDEUI) $(LIB_KIO)
#kde-system-monitor_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#kpm_SOURCES = kpm.c
#
#appdatadir = $(kde_datadir)/kde-system-monitor
#appdata_DATA = ProcessTable.sgrd SystemLoad.sgrd kde-system-monitor-applet.xml
#
## This stuff is now for the kicker applet
#kde_module_LTLIBRARIES = sysguard_panelapplet.la
#
#sysguard_panelapplet_la_SOURCES = \
#	kde-system-monitor-applet.cc \
#	kde-system-monitor-applet-settings.cc
#
#sysguard_panelapplet_la_LIBADD = \
#	ksgrd/libksgrd.la \
#	SensorDisplayLib/libsensordisplays.la \
#	$(top_builddir)/kicker/libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO)
#sysguard_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#EXTRA_DIST = $(lnk_DATA)
#
## just to make sure, automake makes them 
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name "*.ui"` >> rc.cpp
#	$(EXTRACTRC) `find . -name "*.sgrd"` >> rc.cpp
#	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.cc"` -o $(podir)/kde-system-monitor.pot
