kde4_header()

project(kdesktop)

# used by kdesktop but also by lock/
set( kdesktop_KCFG_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/kdesktopsettings.kcfgc
	${CMAKE_CURRENT_SOURCE_DIR}/klaunchsettings.kcfgc )

kde4_add_kcfg_files(kdesktop_LIB_SRCS ${kdesktop_KCFG_SRCS})
include_directories(
	${CMAKE_SOURCE_DIR}/workspace/lib
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background
	${CMAKE_SOURCE_DIR}/libkonq
	${CMAKE_SOURCE_DIR}/workspace/kdmlib
	${KDE4_INCLUDES} )


########### next target ###############

set(kcheckrunning_SRCS kcheckrunning.cpp )

kde4_automoc(${kcheckrunning_SRCS})

kde4_add_executable(kcheckrunning ${kcheckrunning_SRCS})

target_link_libraries(kcheckrunning  ${KDE4_KDECORE_LIBS} )

install(TARGETS kcheckrunning DESTINATION bin)


########### next target ###############

set(kdesktop_SRCS ${kdesktop_LIB_SRCS} ${kdmlib_SRCS} ${backgroundlib_SRCS}
   main.cc
   krootwm.cc
   xautolock.cc
   kdiconview.cc
   desktop.cc
   lockeng.cc
   bgmanager.cc
   init.cc
   minicli.cpp
   pixmapserver.cc
   kcustommenu.cc
   startupid.cpp
   xautolock_diy.c
   xautolock_engine.c
   kshadowengine.cpp
   kshadowsettings.cpp
   kdesktopshadowsettings.cpp
   kfileividesktop.cpp )

kde4_automoc(${kdesktop_SRCS})

kde4_add_ui_files(kdesktop_SRCS minicli.ui )

kde4_add_dcop_skels(kdesktop_SRCS KDesktopIface.h KScreensaverIface.h KBackgroundIface.h )

kde4_add_kdeinit_executable( kdesktop ${kdesktop_SRCS})

target_link_libraries(kdeinit_kdesktop  ${KDE4_KDE3SUPPORT_LIBS} kworkspace ${KDE4_KDESU_LIBS} konq ${X11_X11_LIB} ${X11_Xext_LIB} ${X11_Xss_LIB})
if(X11_Xcursor_FOUND)
  target_link_libraries(kdeinit_kdesktop ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

install_targets(${LIB_INSTALL_DIR} kdeinit_kdesktop )

target_link_libraries( kdesktop kdeinit_kdesktop )
install(TARGETS kdesktop DESTINATION bin)

########### subdirs ###############

add_subdirectory( lock )
add_subdirectory( patterns )
add_subdirectory( programs )
add_subdirectory( init )
add_subdirectory( kwebdesktop )

########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES kdesktop.kcfg klaunch.kcfg )
install_files( /share/autostart FILES kdesktop.desktop )
install_files( ${DATA_INSTALL_DIR}/konqueror/servicemenus FILES kdesktopSetAsBackground.desktop )
install_files( /include FILES KDesktopIface.h KScreensaverIface.h KBackgroundIface.h )
install_files( ${CONFIG_INSTALL_DIR} FILES kdesktop_custom_menu1 kdesktop_custom_menu2 )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am of kdebase/kdesktop
#
#INCLUDES = -I$(top_srcdir)/workspace/lib -I$(top_srcdir)/workspace/kcontrol/background -I$(top_srcdir)/libkonq -I$(top_srcdir)/kdmlib $(all_includes)
#kdesktop_la_LDFLAGS  = $(all_libraries) -module $(KDE_PLUGIN)
#kdesktop_la_LIBADD   = libkdesktopsettings.la $(top_builddir)/libkonq/libkonq.la $(top_builddir)/workspace/kcontrol/background/libbgnd.la $(top_builddir)/kdmlib/libdmctl.la $(LIB_XSS) $(top_builddir)/workspace/lib/libkworkspace.la
#
#SUBDIRS = . lock patterns programs init kwebdesktop
#
######## Files
#
#bin_PROGRAMS = kcheckrunning
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kdesktop.la
#noinst_LTLIBRARIES = libkdesktopsettings.la
#
#libkdesktopsettings_la_LDFLAGS = $(all_libraries) -no-undefined
#libkdesktopsettings_la_LIBADD = $(LIB_KDECORE)
#libkdesktopsettings_la_SOURCES = kdesktopsettings.kcfgc klaunchsettings.kcfgc
#
#kdesktop_la_SOURCES = main.cc krootwm.cc xautolock.cc kdiconview.cc desktop.cc \
#	lockeng.cc KDesktopIface.skel \
#	bgmanager.cc init.cc KScreensaverIface.skel \
#	minicli.cpp KBackgroundIface.skel pixmapserver.cc kcustommenu.cc \
#	startupid.cpp minicli_ui.ui xautolock_diy.c xautolock_engine.c \
#	kshadowengine.cpp kshadowsettings.cpp \
#	kdesktopshadowsettings.cpp kfileividesktop.cpp
#
#include_HEADERS = KDesktopIface.h KScreensaverIface.h KBackgroundIface.h
#
#noinst_HEADERS = desktop.h bgmanager.h krootwm.h \
#	xautolock.h lockeng.h init.h minicli.h \
#	pixmapserver.h startupid.h xautolock_c.h
#
#kcheckrunning_SOURCES = kcheckrunning.cpp
#kcheckrunning_LDFLAGS = $(all_libraries)
#kcheckrunning_LDADD = $(LIB_X11)
#
#METASOURCES = AUTO
#
######## Build rules
#
## List made with: grep -l kdesktopsettings.h *[pc] | sed -e 's/\.c.*$/.lo: kdesktopsettings.h/'
#bgmanager.lo: kdesktopsettings.h
#desktop.lo: kdesktopsettings.h
#init.lo: kdesktopsettings.h
#kdesktopsettings.lo: kdesktopsettings.h
#kdiconview.lo: kdesktopsettings.h
#krootwm.lo: kdesktopsettings.h
#lockeng.lo: kdesktopsettings.h
#main.lo: kdesktopsettings.h
#minicli.lo: kdesktopsettings.h
#
## List made with: grep -l klaunchsettings.h *[pc] | sed -e 's/\.c.*$/.lo: klaunchsettings.h/'
#desktop.lo: klaunchsettings.h
#klaunchsettings.lo: klaunchsettings.h
#startupid.lo: klaunchsettings.h
#
#messages: rc.cpp
#	$(EXTRACTRC) lock/*.ui >> rc.cpp
#	$(XGETTEXT) lock/*.cc *.cc *.cpp *.h -o $(podir)/kdesktop.pot
#
#menu_DATA = kdesktop_custom_menu1 kdesktop_custom_menu2
#menudir = $(kde_confdir)
#
#autostart_DATA = kdesktop.desktop
#autostartdir = $(datadir)/autostart
#
#konqservice_DATA = kdesktopSetAsBackground.desktop
#konqservicedir = $(kde_datadir)/konqueror/servicemenus
#
#kde_kcfg_DATA = kdesktop.kcfg klaunch.kcfg
