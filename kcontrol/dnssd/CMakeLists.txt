kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kdnssd_PART_SRCS kcmdnssd.cpp kcmdnssd.h )

kde4_automoc(${kcm_kdnssd_PART_SRCS})

kde4_add_ui_files(kcm_kdnssd_PART_SRCS configdialog.ui )

kde4_add_plugin(kcm_kdnssd ${kcm_kdnssd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdnssd )

target_link_libraries(kcm_kdnssd  ${QT_AND_KDECORE_LIBS} kdnssd kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kdnssd )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcm_kdnssd.desktop )

kde4_footer()



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
