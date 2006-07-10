
add_subdirectory( pics ) 

macro_bool_to_01(LIBUSB_FOUND HAVE_LIBUSB)
configure_file (config-kcontrol-input.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcontrol-input.h )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/xcursor/ ${KDE4_INCLUDES} )

if (LIBUSB_FOUND)
  include_directories( ${LIBUSB_INCLUDE_DIR} )
endif (LIBUSB_FOUND)

if(X11_Xcursor_FOUND)
	set( libnoinst_SRCS xcursor/themepage.cpp xcursor/previewwidget.cpp )
else(X11_Xcursor_FOUND)
	set( libnoinst_SRCS core/themepage.cpp )
endif(X11_Xcursor_FOUND)



########### next target ###############

set(kapplymousetheme_SRCS kapplymousetheme.cpp )

kde4_automoc(${kapplymousetheme_SRCS})

kde4_add_executable(kapplymousetheme ${kapplymousetheme_SRCS})

target_link_libraries(kapplymousetheme  ${KDE4_KDE3SUPPORT_LIBS} ${X11_Xrender_LIB})
if (X11_Xcursor_FOUND)
   target_link_libraries(kapplymousetheme ${X11_Xcursor_LIB})
endif (X11_Xcursor_FOUND)

install(TARGETS kapplymousetheme DESTINATION bin)


########### next target ###############

set(kcm_input_PART_SRCS mouse.cpp main.cpp logitechmouse.cpp ${libnoinst_SRCS})

kde4_automoc(${kcm_input_PART_SRCS})

kde4_add_ui3_files(kcm_input_PART_SRCS kmousedlg.ui logitechmouse_base.ui )

kde4_add_plugin(kcm_input ${kcm_input_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_input )

target_link_libraries(kcm_input ${KDE4_KDE3SUPPORT_LIBS})
if (LIBUSB_FOUND)
   target_link_libraries(kcm_input ${LIBUSB_LIBRARIES})
endif (LIBUSB_FOUND)
if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_input ${X11_Xcursor_LIB})
endif (X11_Xcursor_FOUND)
if (X11_Xrender_FOUND)
   target_link_libraries(kcm_input ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)

install(TARGETS kcm_input  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES mouse.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz  DESTINATION  ${DATA_INSTALL_DIR}/kcminput )
install( FILES mouse_cursor_theme.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




#original Makefile.am contents follow:

#if have_xcursor
#THEMEPAGE_SUBDIR = xcursor
#THEMEPAGE_INCLUDES = -I$(srcdir)/xcursor
#THEMEPAGE_LIB = xcursor/libthemepage.la
#else
#THEMEPAGE_SUBDIR = core
#THEMEPAGE_INCLUDES = -I$(srcdir)/core
#THEMEPAGE_LIB = core/libthemepage.la
#endif
#
#SUBDIRS = $(THEMEPAGE_SUBDIR) pics
#
#AM_CPPFLAGS  = $(USB_CFLAGS) $(THEMEPAGE_INCLUDES) $(all_includes)
#
#bin_PROGRAMS = kapplymousetheme
#
#kapplymousetheme_SOURCES = kapplymousetheme.cpp
#kapplymousetheme_LDFLAGS = $(all_libraries)
#kapplymousetheme_LDADD = $(LIB_XCURSOR) $(LIB_X11)
#
#kde_module_LTLIBRARIES = kcm_input.la
#
#kcm_input_la_SOURCES = mouse.cpp kmousedlg.ui main.cpp logitechmouse.cpp logitechmouse_base.ui
#kcm_input_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_input_la_LIBADD = $(THEMEPAGE_LIB) $(XTESTLIB) $(LIB_KIO) $(LIB_XCURSOR) $(USB_LIBS)
#
#METASOURCES = AUTO 
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp */*.cpp -o $(podir)/kcminput.pot
#
#xdg_apps_DATA = mouse.desktop
#
#fontdir = $(kde_datadir)/kcminput
#font_DATA = cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz
#EXTRA_DIST = $(font_DATA)
#
#update_DATA = mouse_cursor_theme.upd
#
#updatedir = $(kde_datadir)/kconf_update
#
#install-data-hook:
#	$(mkinstalldirs) $(DESTDIR)$(datadir)/fonts/override
#	-mkfontdir $(DESTDIR)$(datadir)/fonts/override
###I'd like to install this automatically, but it only works on Linux, and I don't want to break things...
### $(INSTALL_DATA) $(srcdir)/consoleUserPerms $(DESTDIR)/etc/hotplug/usb/consoleUserPerms
### $(INSTALL_DATA) $(srcdir)/logitechmouse.usermap $(DESTDIR)/etc/hotplug/usb/logitechmouse.usermap
#KDE_ICON=AUTO
