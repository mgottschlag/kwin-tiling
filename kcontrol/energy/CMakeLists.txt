kde4_header()

add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_energy_PART_SRCS energy.cpp )

kde4_automoc(${kcm_energy_PART_SRCS})

kde4_add_plugin(kcm_energy ${kcm_energy_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_energy )

target_link_libraries(kcm_energy  ${LIB_KIO} )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_energy )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/.hidden FILES energy.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS= . pics
#
#AM_CPPFLAGS = $(all_includes) $(DPMSINC)
#
#kde_module_LTLIBRARIES = kcm_energy.la
#
#kcm_energy_la_SOURCES = energy.cpp
#kcm_energy_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
#kcm_energy_la_LIBADD = $(DPMSLIB) $(LIB_KDEUI) $(LIB_KIO)
#METASOURCES = energy.moc
#
#noinst_HEADERS = energy.h
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmenergy.pot
#
#energy_data_DATA = energy.desktop
#energy_datadir = $(kde_appsdir)/.hidden
#KDE_ICON=AUTO
