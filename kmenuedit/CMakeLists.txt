
project(kmenuedit)

add_subdirectory( pixmaps ) 



set(kmenueditcommon_STAT_SRCS basictab.cpp treeview.cpp kmenuedit.cpp khotkeys.cpp menufile.cpp menuinfo.cpp )

########### next target ###############

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

kde4_automoc(${kmenuedit_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmenuedit  ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTXML_LIBRARY}
)

install(TARGETS kdeinit_kmenuedit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmenuedit kdeinit_kmenuedit )
install(TARGETS kmenuedit DESTINATION bin)

########### next target ###############

set(kcontroledit_KDEINIT_SRCS kcontrol_main.cpp ${kmenueditcommon_STAT_SRCS})

kde4_automoc(${kcontroledit_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcontroledit ${kcontroledit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcontroledit  ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTXML_LIBRARY} )

install(TARGETS kdeinit_kcontroledit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kcontroledit kdeinit_kcontroledit )
install(TARGETS kcontroledit DESTINATION bin)

########### next target ###############


########### install files ###############

install( FILES kmenuedit.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kcontroleditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kcontroledit )
install( FILES kmenueditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmenuedit )


kde4_install_icons( ${ICON_INSTALL_DIR} hicolor )




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kmenuedit.la kcontroledit.la
#
#noinst_LTLIBRARIES = libkmenueditcommon.la
#
#CLEANFILES = dummy.cpp
#
#libkmenueditcommon_la_SOURCES = basictab.cpp treeview.cpp kmenuedit.cpp \
#	khotkeys.cpp menufile.cpp menuinfo.cpp
#
#libkmenueditcommon_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDE3SUPPORT)
#libkmenueditcommon_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#kmenuedit_la_SOURCES = main.cpp
#kmenuedit_la_LIBADD = libkmenueditcommon.la
#kmenuedit_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#kcontroledit_la_SOURCES = kcontrol_main.cpp
#kcontroledit_la_LIBADD = libkmenueditcommon.la
#kcontroledit_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#noinst_HEADERS = kmenuedit.h treeview.h basictab.h khotkeys.h \
#	menufile.h menuinfo.h
#
#METASOURCES = AUTO
#
#xdg_apps_DATA = kmenuedit.desktop
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/System
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/System/kmenuedit.desktop
#
#KDE_ICON = kmenuedit
#
#EXTRA_DIST = $(xdg_apps_DATA)
#
#rcdir = $(kde_datadir)/kmenuedit
#rc_DATA = kmenueditui.rc
#
#rc2dir = $(kde_datadir)/kcontroledit
#rc2_DATA = kcontroleditui.rc
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kmenuedit.pot
#
#dummy.cpp:
#	echo > dummy.cpp
#
#SUBDIRS = pixmaps
