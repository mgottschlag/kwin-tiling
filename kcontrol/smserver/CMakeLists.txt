kde4_header()
include_directories( ${KDE4_INCLUDES} )

########### next target ###############

set(kcm_smserver_PART_SRCS kcmsmserver.cpp smserverconfigimpl.cpp )

kde4_automoc(${kcm_smserver_PART_SRCS})

kde4_add_ui3_files(kcm_smserver_PART_SRCS smserverconfigdlg.ui )

kde4_add_plugin(kcm_smserver ${kcm_smserver_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_smserver )

target_link_libraries(kcm_smserver  ${KDE4_KDE3SUPPORT_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kcm_smserver )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmsmserver.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_smserver.la
#
#kcm_smserver_la_SOURCES = kcmsmserver.cpp smserverconfigdlg.ui smserverconfigimpl.cpp
#kcm_smserver_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_smserver_la_LIBADD = $(top_builddir)/workspace/lib/libkworkspace.la -lkdeui
#AM_CPPFLAGS= -I$(top_srcdir)/workspace/lib $(all_includes) 
#
#METASOURCES = AUTO
#
#noinst_HEADERS = kcmsmserver.h smserverconfigimpl.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmsmserver.pot
#
#xdg_apps_DATA = kcmsmserver.desktop
#
#EXTRA_DIST = $(xdg_apps_DATA)
