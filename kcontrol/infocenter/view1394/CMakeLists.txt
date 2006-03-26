kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(kcm_view1394_PART_SRCS view1394.cpp )

kde4_automoc(${kcm_view1394_PART_SRCS})

kde4_add_ui3_files(kcm_view1394_PART_SRCS view1394widget.ui )

kde4_add_plugin(kcm_view1394 ${kcm_view1394_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_view1394 )

target_link_libraries(kcm_view1394  ${KDE4_KDE3SUPPORT_LIBS} raw1394 )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_view1394 )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmview1394.desktop )
install_files( ${DATA_INSTALL_DIR}/kcmview1394 FILES oui.db )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_view1394.la
#
#kcm_view1394_la_SOURCES = view1394widget.ui view1394.cpp
#
#kcm_view1394_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_view1394_la_LIBADD = $(LIB_KDEUI) -lraw1394
#
#AM_CPPFLAGS= $(all_includes) 
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmview1394.pot
#
#xdg_apps_DATA = kcmview1394.desktop
#
#view1394_vendor_DATA = oui.db
#view1394_vendordir = $(kde_datadir)/kcmview1394
#
