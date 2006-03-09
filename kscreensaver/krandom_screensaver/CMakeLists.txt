kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kscreensaver/libkscreensaver ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(krandom.kss_SRCS random.cpp )

kde4_automoc(${krandom.kss_SRCS})

kde4_add_executable(krandom.kss ${krandom.kss_SRCS})

target_link_libraries(krandom.kss  ${LIB_KDEUI}  m )

install_targets(/bin krandom.kss )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/System/ScreenSavers FILES krandom.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Makefile.am for krandom screensaver.
##
#
#AM_CPPFLAGS = -UQT_NO_ASCII_CAST
#
#INCLUDES = -I$(top_srcdir)/workspace/kscreensaver/libkscreensaver $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#bin_PROGRAMS = krandom.kss
#
#krandom_kss_SOURCES = random.cpp
#krandom_kss_LDADD = $(LIB_KDEUI) -lm
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kscreensaver.pot
#
########### Meta objects ##########
#
#desktop_DATA = krandom.desktop
#desktopdir = $(kde_appsdir)/System/ScreenSavers
#
#EXTRA_DIST = $(desktop_DATA)
