


########### next target ###############

set(kstyle_keramik_config_PART_SRCS keramikconf.cpp )

kde4_automoc(${kstyle_keramik_config_PART_SRCS})

kde4_add_plugin(kstyle_keramik_config ${kstyle_keramik_config_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kstyle_keramik_config )

target_link_libraries(kstyle_keramik_config  ${KDE4_KDEUI_LIBS} )

install(TARGETS kstyle_keramik_config  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_HEADERS = keramikconf.h
#kde_module_LTLIBRARIES = kstyle_keramik_config.la
#kstyle_keramik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#kstyle_keramik_config_la_LIBADD  = $(LIB_KDEUI)
#kstyle_keramik_config_la_SOURCES = keramikconf.cpp
#METASOURCES = AUTO 
#
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_keramik_config.pot
