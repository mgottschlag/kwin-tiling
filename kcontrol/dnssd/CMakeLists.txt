


########### next target ###############

set(kcm_kdnssd_PART_SRCS kcmdnssd.cpp kcmdnssd.h )

kde4_automoc(${kcm_kdnssd_PART_SRCS})

kde4_add_ui_files(kcm_kdnssd_PART_SRCS configdialog.ui )

kde4_add_plugin(kcm_kdnssd ${kcm_kdnssd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdnssd )

target_link_libraries(kcm_kdnssd  ${KDE4_KDECORE_LIBS} kdnssd kdeui )

install(TARGETS kcm_kdnssd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcm_kdnssd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES =  kcm_kdnssd.la
#kcm_kdnssd_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)
#kcm_kdnssd_la_SOURCES = configdialog.ui kcmdnssd.cpp kcmdnssd.h
#
#xdg_apps_DATA = kcm_kdnssd.desktop
#kcm_kdnssd_la_LIBADD = $(LIB_KDNSSD) $(LIB_KDEUI)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kcmkdnssd.pot
