kde4_header()

add_subdirectory( default ) 
add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_locale_PART_SRCS 
   main.cpp 
   toplevel.cpp 
   kcmlocale.cpp 
   klocalesample.cpp 
   localenum.cpp 
   localemon.cpp 
   localetime.cpp 
   localeother.cpp )

kde4_automoc(${kcm_locale_PART_SRCS})

kde4_add_plugin(kcm_locale ${kcm_locale_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_locale )

target_link_libraries(kcm_locale  ${KDE4_KDECORE_LIBS} kio ${QT_QTGUI_LIBRARY} kdeui ${QT_QT3SUPPORT_LIBRARY})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_locale )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES language.desktop )

kde4_footer()



#original Makefile.am contents follow:

##AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_COMPAT
#
#AM_CPPFLAGS = $(all_includes)
#
#SUBDIRS = default pics
#
#kde_module_LTLIBRARIES = kcm_locale.la
#kcm_locale_la_SOURCES = main.cpp toplevel.cpp kcmlocale.cpp klocalesample.cpp localenum.cpp localemon.cpp localetime.cpp localeother.cpp
#METASOURCES = AUTO
#noinst_HEADERS = toplevel.h kcmlocale.h klocalesample.h localenum.h localemon.h localetime.h localeother.h 
#
#kcm_locale_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
#kcm_locale_la_LIBADD = $(LIB_KIO)
#
## Note: newer glibc (e.g. zonetab version 1.31 (cvs revision 1.22) have more/other data than older ones (e.g. zonetab version 1.28)
#
## timezones-manual is for adding entried from a zone.tab downloaded from glibc's CVS
#timezones-manual:
#	grep -v '^#' zone.tab | tr '	' ' ' | sed -e "s#  *# #" | cut -d' ' -f3 | sed -e 's,^,i18n(",; s,$$,");,' >> $(srcdir)/TIMEZONES
#	sort -u $(srcdir)/TIMEZONES > t && mv t $(srcdir)/TIMEZONES
#
#timezones:
#	grep -v '^#' /usr/share/zoneinfo/zone.tab | tr '	' ' ' | sed -e "s#  *# #" | cut -d' ' -f3 | sed -e 's,^,i18n(",; s,$$,");,' >> $(srcdir)/TIMEZONES
#	sort -u $(srcdir)/TIMEZONES > t && mv t $(srcdir)/TIMEZONES
#	
#
#messages:
#	$(XGETTEXT) -ktranslate:1,1t -ktranslate:1c,2,2t *.cpp -o $(podir)/kcmlocale.pot
#	$(XGETTEXT) TIMEZONES -o $(podir)/../kdelibs/timezones.pot
#
#xdg_apps_DATA = language.desktop
#
