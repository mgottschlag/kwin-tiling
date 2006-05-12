
add_subdirectory( pics ) 
include_directories( ${CMAKE_SOURCE_DIR}/libkonq   )


########### next target ###############
kde4_add_ui3_files(kcmbackgroundlib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/bgdialog_ui.ui ${CMAKE_CURRENT_SOURCE_DIR}/bgwallpaper_ui.ui ${CMAKE_CURRENT_SOURCE_DIR}/bgadvanced_ui.ui )


set(kcm_background_PART_SRCS main.cpp ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS})

kde4_automoc(${kcm_background_PART_SRCS})

kde4_add_plugin(kcm_background ${kcm_background_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_background )

target_link_libraries(kcm_background  ${KDE4_KDE3SUPPORT_LIBS} knewstuff kdefx ${X11_X11_LIB})

install_targets(${PLUGIN_INSTALL_DIR} kcm_background )


########### next target ###############


########### next target ###############


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES background.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




#original Makefile.am contents follow:

#SUBDIRS = . pics
#
#AM_CPPFLAGS = -I$(top_srcdir)/libkonq $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_background.la
#
#kcm_background_la_SOURCES = main.cpp
#kcm_background_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
#kcm_background_la_LIBADD = libkcmbgnd.la libbgnd.la $(LIB_KFILE) $(LIB_KNEWSTUFF) $(LIB_KDE3SUPPORT)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = main.h \
# bgrender.h bgsettings.h \
# bgdialog.h bgadvanced.h bgwallpaper.h bgdefaults.h bgmonitor.h
#
#noinst_LTLIBRARIES = libbgnd.la libkcmbgnd.la
#libbgnd_la_SOURCES = bgrender.cpp bgsettings.cpp
#libkcmbgnd_la_SOURCES = bgdialog.cpp bgdialog_ui.ui bgwallpaper_ui.ui \
#                bgwallpaper.cpp bgadvanced_ui.ui bgadvanced.cpp bgmonitor.cpp
#
#main.lo: bgdialog_ui.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmbackground.pot
#
#xdg_apps_DATA = background.desktop
#
#KDE_ICON=AUTO
