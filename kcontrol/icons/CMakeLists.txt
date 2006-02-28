kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_icons_PART_SRCS iconthemes.cpp icons.cpp main.cpp )

kde4_automoc(${kcm_icons_PART_SRCS})

kde4_add_plugin(kcm_icons ${kcm_icons_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_icons )

target_link_libraries(kcm_icons  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_icons )


########### next target ###############

set(testicons_SRCS testicons.cpp )

kde4_automoc(${testicons_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testicons ${testicons_SRCS})

target_link_libraries(testicons  ${QT_AND_KDECORE_LIBS} kdeui )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${XDG_APPS_DIR} FILES icons.desktop )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS  = $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_icons.la
#
#kcm_icons_la_SOURCES = iconthemes.cpp icons.cpp main.cpp
#kcm_icons_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
#kcm_icons_la_LIBADD = $(LIB_KIO)
#
#METASOURCES = AUTO 
#
#noinst_HEADERS = icons.h iconthemes.h main.h
#
#check_PROGRAMS = testicons
#testicons_SOURCES = testicons.cpp 
#testicons_LDFLAGS = $(all_libraries)
#testicons_LDADD = $(LIB_KDEUI) icons.lo
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmicons.pot
#
#xdg_apps_DATA = icons.desktop
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/Themes
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/Themes/iconthemes.desktop
#
#KDE_ICON=AUTO
