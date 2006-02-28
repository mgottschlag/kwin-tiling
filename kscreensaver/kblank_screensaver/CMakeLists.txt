kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kscreensaver/libkscreensaver ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kblankscrn.kss_SRCS blankscrn.cpp )

kde4_automoc(${kblankscrn.kss_SRCS})

kde4_add_executable(kblankscrn.kss ${kblankscrn.kss_SRCS})

target_link_libraries(kblankscrn.kss  ${QT_AND_KDECORE_LIBS} kdeui kscreensaver m )

install_targets(/bin kblankscrn.kss )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/System/ScreenSavers FILES kblank.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Makefile.am for kblank screensaver.
##
#
#AM_CPPFLAGS = -UQT_NO_ASCII_CAST
#
#INCLUDES = -I$(top_srcdir)/workspace/kscreensaver/libkscreensaver $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#bin_PROGRAMS = kblankscrn.kss
#
#kblankscrn_kss_SOURCES = blankscrn.cpp
#kblankscrn_kss_LDADD = $(LIB_KDEUI) $(top_builddir)/workspace/kscreensaver/libkscreensaver/libkscreensaver.la -lm 
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kscreensaver.pot
#
########### Meta objects ##########
#
#desktop_DATA = kblank.desktop
#desktopdir = $(kde_appsdir)/System/ScreenSavers
#
#EXTRA_DIST = $(desktop_DATA)
