kde4_header()

project(ktip)

add_subdirectory( pics ) 
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(ktip_SRCS ktipwindow.cpp )

kde4_automoc(${ktip_SRCS})

kde4_add_executable(ktip ${ktip_SRCS})

target_link_libraries(ktip  ${KDE4_KDEUI_LIBS})

install_targets(/bin ktip )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES ktip.desktop )
install_files( /share/autostart FILES ktip.desktop )
install_files( ${DATA_INSTALL_DIR}/kdewizard FILES tips )


install(FILES uninstall.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}${APPLNK_INSTALL_DIR}/Toys/ RENAME ktip.desktop)


kde4_install_icons( ${ICON_INSTALL_DIR} hicolor )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#SUBDIRS = pics
#
#bin_PROGRAMS = ktip
#ktip_SOURCES = ktipwindow.cpp
#ktip_LDADD = $(LIB_KDEUI)
#ktip_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#METASOURCES = AUTO
#KDE_ICON = AUTO
#
#xdg_apps_DATA = ktip.desktop
#
#autostart_DATA = ktip.desktop
#autostartdir = $(datadir)/autostart
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Toys
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Toys/ktip.desktop
#
#messages: 
#	$(PREPARETIPS) > tips.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/ktip.pot
#	rm -f tips.cpp
#
#appdata_DATA = tips
#appdatadir=$(kde_datadir)/kdewizard
#
