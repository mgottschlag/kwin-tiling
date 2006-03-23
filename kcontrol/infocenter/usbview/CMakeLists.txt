kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kcm_usb_PART_SRCS kcmusb.cpp usbdevices.cpp usbdb.cpp )

kde4_automoc(${kcm_usb_PART_SRCS})

kde4_add_plugin(kcm_usb ${kcm_usb_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_usb )

target_link_libraries(kcm_usb  ${KDE4_KDECORE_LIBS} kdeui ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_usb )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmusb.desktop )
install_files( ${DATA_INSTALL_DIR}/kcmusb FILES usb.ids )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_usb.la
#
#kcm_usb_la_SOURCES = kcmusb.cpp usbdevices.cpp usbdb.cpp
#
#kcm_usb_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_usb_la_LIBADD = $(LIB_KDEUI)
#
#AM_CPPFLAGS= $(all_includes) 
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp *.i18n -o $(podir)/kcmusb.pot
#
#xdg_apps_DATA = kcmusb.desktop
#
#usb_DATA = usb.ids
#usbdir = $(kde_datadir)/kcmusb
#
