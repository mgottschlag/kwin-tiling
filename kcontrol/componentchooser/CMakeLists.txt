kde4_header()

add_subdirectory( componentservices ) 
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(kcm_componentchooser_PART_SRCS componentchooser.cpp kcm_componentchooser.cpp )

kde4_automoc(${kcm_componentchooser_PART_SRCS})

kde4_add_ui3_files(kcm_componentchooser_PART_SRCS 
   componentchooser_ui.ui 
   componentconfig_ui.ui 
   emailclientconfig_ui.ui 
   terminalemulatorconfig_ui.ui 
   browserconfig_ui.ui )

kde4_add_plugin(kcm_componentchooser ${kcm_componentchooser_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_componentchooser )

target_link_libraries(kcm_componentchooser  ${KDE4_KDE3SUPPORT_LIBS})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_componentchooser )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES componentchooser.desktop )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS= . componentservices
#AM_CPPFLAGS  = -I../ -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)
#
#METASOURCES = AUTO 
#
#kde_module_LTLIBRARIES = kcm_componentchooser.la
#
#kcm_componentchooser_la_SOURCES = componentchooser_ui.ui componentconfig_ui.ui emailclientconfig_ui.ui terminalemulatorconfig_ui.ui browserconfig_ui.ui componentchooser.cpp kcm_componentchooser.cpp
#kcm_componentchooser_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_componentchooser_la_LIBADD =  $(LIB_KIO)
#
#xdg_apps_DATA = componentchooser.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmcomponentchooser.pot
