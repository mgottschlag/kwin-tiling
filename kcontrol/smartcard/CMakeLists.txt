kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kcm_smartcard_PART_SRCS smartcard.cpp )

kde4_automoc(${kcm_smartcard_PART_SRCS})

kde4_add_ui_files(kcm_smartcard_PART_SRCS smartcardbase.ui nosmartcardbase.ui )

kde4_add_dcop_skels(kcm_smartcard_PART_SRCS smartcard.h )

kde4_add_plugin(kcm_smartcard ${kcm_smartcard_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_smartcard )

target_link_libraries(kcm_smartcard  ${KDE4_KDECORE_LIBS} kdeui ksmartcard kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_smartcard )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES smartcard.desktop )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS  = $(all_includes)
#kde_module_LTLIBRARIES = kcm_smartcard.la
#
#kcm_smartcard_la_SOURCES = smartcard.cpp smartcardbase.ui nosmartcardbase.ui smartcard.skel
#
#kcm_smartcard_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
#
#kcm_smartcard_la_LIBADD = -lkdeui -lksmartcard $(LIB_KIO)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = smartcard.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmsmartcard.pot
#
#xdg_apps_DATA = smartcard.desktop
