kde4_header()

add_subdirectory( keramik ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_style_PART_SRCS kcmstyle.cpp menupreview.cpp styleconfdialog.cpp )

kde4_automoc(${kcm_style_PART_SRCS})

kde4_add_ui3_files(kcm_style_PART_SRCS stylepreview.ui )

kde4_add_plugin(kcm_style ${kcm_style_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_style )

target_link_libraries(kcm_style  ${QT_AND_KDECORE_LIBS} kio krdb )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_style )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES style.desktop )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#
#AM_CPPFLAGS  = $(all_includes)
#SUBDIRS = keramik . 
#
#kde_module_LTLIBRARIES = kcm_style.la
#
#kcm_style_la_SOURCES = kcmstyle.cpp stylepreview.ui menupreview.cpp styleconfdialog.cpp
#kcm_style_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
#kcm_style_la_LIBADD = $(LIB_KIO) ../krdb/libkrdb.la
#METASOURCES = AUTO 
#
#noinst_HEADERS = kcmstyle.h stylepreview.h menupreview.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmstyle.pot
#
#xdg_apps_DATA = style.desktop
#
#KDE_ICON=AUTO
