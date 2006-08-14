


########### next target ###############

set(kcm_arts_PART_SRCS arts.cpp krichtextlabel.cpp )

kde4_automoc(${kcm_arts_PART_SRCS})

kde4_add_ui3_files(kcm_arts_PART_SRCS generaltab.ui hardwaretab.ui )

kde4_add_plugin(kcm_arts ${kcm_arts_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_arts )

target_link_libraries(kcm_arts  ${KDE4_KIO_LIBS} kutils kmid )

install(TARGETS kcm_arts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES arts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )




#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_arts.la
#
#kcm_arts_la_SOURCES = arts.cpp generaltab.ui hardwaretab.ui krichtextlabel.cpp
#
#kcm_arts_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_arts_la_LIBADD = $(LIB_KIO) $(LIB_KUTILS) -lkmid
#
#AM_CPPFLAGS = -I$(kde_includes)/arts  $(all_includes)
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmarts.pot
#
#xdg_apps_DATA = arts.desktop
#
#KDE_ICON=AUTO
