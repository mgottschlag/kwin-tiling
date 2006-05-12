kde4_header()



########### next target ###############

set(kcm_samba_PART_SRCS 
   main.cpp 
   ksmbstatus.cpp 
   kcmsambaimports.cpp 
   kcmsambalog.cpp 
   kcmsambastatistics.cpp )

kde4_automoc(${kcm_samba_PART_SRCS})

kde4_add_plugin(kcm_samba ${kcm_samba_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_samba )

target_link_libraries(kcm_samba  ${KDE4_KDE3SUPPORT_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kcm_samba )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES smbstatus.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_samba.la
#
#kcm_samba_la_SOURCES = main.cpp ksmbstatus.cpp kcmsambaimports.cpp kcmsambalog.cpp kcmsambastatistics.cpp
#
#kcm_samba_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_samba_la_LIBADD = $(LIB_KIO)
#
#AM_CPPFLAGS = $(all_includes)
#
#METASOURCES = AUTO 
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmsamba.pot
#
#xdg_apps_DATA = smbstatus.desktop
#KDE_ICON=AUTO
