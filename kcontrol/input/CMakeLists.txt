kde4_header()

add_subdirectory( pics ) 
if(X11_Xcursor_FOUND)
	add_subdirectory( xcursor )
	include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/xcursor/ )
else(X11_Xcursor_FOUND)
	add_subdirectory( core )
	include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/core/ )
endif(X11_Xcursor_FOUND)



########### next target ###############

set(kapplymousetheme_SRCS kapplymousetheme.cpp )

kde4_automoc(${kapplymousetheme_SRCS})

kde4_add_executable(kapplymousetheme ${kapplymousetheme_SRCS})

target_link_libraries(kapplymousetheme  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kapplymousetheme )


########### next target ###############

set(kcm_input_PART_SRCS mouse.cpp main.cpp logitechmouse.cpp )

kde4_automoc(${kcm_input_PART_SRCS})

kde4_add_ui3_files(kcm_input_PART_SRCS kmousedlg.ui logitechmouse_base.ui )

kde4_add_plugin(kcm_input ${kcm_input_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_input )

target_link_libraries(kcm_input  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_input )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES mouse.desktop )
install_files( ${DATA_INSTALL_DIR}/kcminput FILES cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES mouse_cursor_theme.upd )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



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
