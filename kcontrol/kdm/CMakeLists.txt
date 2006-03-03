kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kdm_PART_SRCS ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS} 
   background.cpp 
   kdm-appear.cpp 
   kdm-font.cpp 
   kdm-shut.cpp 
   kdm-users.cpp 
   kdm-conv.cpp 
   main.cpp 
   kbackedcombobox.cpp )

kde4_automoc(${kcm_kdm_PART_SRCS})

kde4_add_plugin(kcm_kdm ${kcm_kdm_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdm )

target_link_libraries(kcm_kdm  ${QT_AND_KDECORE_LIBS} kcmbgnd bgnd kio knewstuff kde3support )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kdm )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kdm.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_kdm.la
#
## _don't_ add a theme configurator!!
#kcm_kdm_la_SOURCES = background.cpp kdm-appear.cpp kdm-font.cpp kdm-shut.cpp \
#	kdm-users.cpp kdm-conv.cpp main.cpp kbackedcombobox.cpp
#
#kcm_kdm_la_LDFLAGS  = $(KDE_RPATH) $(all_libraries) -module -avoid-version -no-undefined
#kcm_kdm_la_LIBADD = $(top_builddir)/workspace/kcontrol/background/libkcmbgnd.la $(top_builddir)/workspace/kcontrol/background/libbgnd.la $(LIB_KIO) $(LIB_KNEWSTUFF) -lkde3support
#
#AM_CPPFLAGS= -I$(top_builddir)/workspace/kcontrol/background \
#          -I$(top_srcdir)/workspace/kcontrol/background \
#          -I$(top_srcdir)/workspace/kdesktop $(all_includes)
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kdmconfig.pot
#
#xdg_apps_DATA = kdm.desktop
#KDE_ICON=AUTO
