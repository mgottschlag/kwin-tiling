kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_arts_PART_SRCS arts.cpp krichtextlabel.cpp )

kde4_automoc(${kcm_arts_PART_SRCS})

kde4_add_ui3_files(kcm_arts_PART_SRCS generaltab.ui hardwaretab.ui )

kde4_add_plugin(kcm_arts ${kcm_arts_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_arts )

target_link_libraries(kcm_arts  ${QT_AND_KDECORE_LIBS} kio kutils kmid )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_arts )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES arts.desktop )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_arts.la
#
#kcm_arts_la_SOURCES = arts.cpp generaltab.ui hardwaretab.ui krichtextlabel.cpp
#
#kcm_arts_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#kcm_arts_la_LIBADD = $(LIB_KIO) $(LIB_KUTILS) -lkmid
#
#AM_CPPFLAGS = -I$(kde_includes)/arts  $(all_includes)
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmarts.pot
#
#xdg_apps_DATA = arts.desktop
#
#KDE_ICON=AUTO
