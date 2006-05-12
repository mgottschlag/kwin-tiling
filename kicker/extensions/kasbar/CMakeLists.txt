


########### next target ###############

set(kasbar_LIB_SRCS 
   kasbar.cpp 
   kasitem.cpp 
   kastaskitem.cpp 
   kastasker.cpp 
   kaspopup.cpp 
   kastaskpopup.cpp 
   kasstartupitem.cpp 
   kasaboutdlg.cpp 
   kasprefsdlg.cpp 
   kasclockitem.cpp 
   kasgroupitem.cpp 
   kasresources.cpp 
   kasgrouper.cpp 
   kasloaditem.cpp )

kde4_automoc(${kasbar_LIB_SRCS})

kde4_add_library(kasbar SHARED ${kasbar_LIB_SRCS})

target_link_libraries(kasbar  ${KDE4_KDEUI_LIBS} taskmanager kasbar kdeui )

set_target_properties(kasbar PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kasbar )


########### next target ###############

set(kasbar_panelextension_PART_SRCS kasbarextension.cpp )

kde4_automoc(${kasbar_panelextension_PART_SRCS})

kde4_add_plugin(kasbar_panelextension ${kasbar_panelextension_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kasbar_panelextension )

target_link_libraries(kasbar_panelextension  ${KDE4_KDECORE_LIBS} kasbar )

install_targets(${PLUGIN_INSTALL_DIR} kasbar_panelextension )


########### next target ###############

set(kasbar_SRCS kasbarapp.cpp )

kde4_automoc(${kasbar_SRCS})

kde4_add_executable(kasbar ${kasbar_SRCS})

target_link_libraries(kasbar  ${KDE4_KDECORE_LIBS} )

install(TARGETS kasbar DESTINATION bin)


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/extensions FILES kasbarextension.desktop )




#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../taskmanager $(all_includes)
#METASOURCES = AUTO
#
##
## Library implementing the core functionality
##
#lib_LTLIBRARIES = libkasbar.la
#
#libkasbar_la_SOURCES =	kasbar.cpp kasitem.cpp \
#			kastaskitem.cpp	kastasker.cpp kaspopup.cpp \
#			kastaskpopup.cpp kasstartupitem.cpp \
#			kasaboutdlg.cpp kasprefsdlg.cpp kasclockitem.cpp \
#			kasgroupitem.cpp kasresources.cpp kasgrouper.cpp \
#			kasloaditem.cpp
#
#libkasbar_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libkasbar_la_LIBADD = $(LIB_KDEUI) ../../taskmanager/libtaskmanager.la
#
##
## Panel Extension
##
#kde_module_LTLIBRARIES = kasbar_panelextension.la
#
#kasbar_panelextension_la_SOURCES =  kasbarextension.cpp
#kasbar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#kasbar_panelextension_la_LIBADD =  libkasbar.la
#
#noinst_HEADERS = kasbar.h kasitem.h kasbarextension.h \
#		kastaskitem.h kastasker.h kaspopup.h kasstartupitem.h \
#		kasaboutdlg.h kasprefsdlg.h version.h kasclockitem.cpp \
#		kasgroupitem.h kasresources.h kasgrouper.h kasloaditem.cpp
#
#lnkdir = $(kde_datadir)/kicker/extensions
#lnk_DATA = kasbarextension.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#bin_PROGRAMS = kasbar
#kasbar_SOURCES = kasbarapp.cpp
#kasbar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kasbar_LDADD = 	libkasbar.la \
#		$(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)
#
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kasbarextension.pot
#
#srcdoc:
#	$(kde_bindir)/kdoc -a -p -d classdocs -n 'Kasbar2' ../../taskmanager/*.h *.h -lqt -lkdecore -lkdeui
#
#.PHONY:	changes
#changes:
#	cvs2cl.pl --window 3600 -w --hide-filenames -I '.desktop'
#
#KDE_OPTIONS = nofinal
