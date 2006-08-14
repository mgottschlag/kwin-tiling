
project(ktip)

add_subdirectory( pics ) 



########### next target ###############

set(ktip_SRCS ktipwindow.cpp )

kde4_automoc(${ktip_SRCS})

kde4_add_executable(ktip ${ktip_SRCS})

target_link_libraries(ktip  ${KDE4_KDEUI_LIBS})

install(TARGETS ktip DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES ktip.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES ktip.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES tips  DESTINATION  ${DATA_INSTALL_DIR}/kdewizard )


kde4_install_icons( ${ICON_INSTALL_DIR}  )




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
