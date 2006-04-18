kde4_header()
include_directories( ${X11_Xinerama_INCLUDE_PATH}  ${KDE4_INCLUDES} )


########### next target ###############

set(kcm_xinerama_PART_SRCS kcmxinerama.cpp )

kde4_automoc(${kcm_xinerama_PART_SRCS})

kde4_add_ui3_files(kcm_xinerama_PART_SRCS xineramawidget.ui )

kde4_add_plugin(kcm_xinerama ${kcm_xinerama_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_xinerama )

target_link_libraries(kcm_xinerama  ${KDE4_KDE3SUPPORT_LIBS} ${X11_Xinerama_LIB})

install_targets(${PLUGIN_INSTALL_DIR} kcm_xinerama )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/.hidden FILES xinerama.desktop )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS  = $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_xinerama.la
#
#kcm_xinerama_la_SOURCES = kcmxinerama.cpp xineramawidget.ui
#
#kcm_xinerama_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_xinerama_la_LIBADD = -lkdeui
#
#METASOURCES = AUTO 
#
#noinst_HEADERS = kcmxinerama.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmxinerama.pot
#
#xinerama_datadir = $(kde_appsdir)/.hidden
#xinerama_data_DATA = xinerama.desktop
