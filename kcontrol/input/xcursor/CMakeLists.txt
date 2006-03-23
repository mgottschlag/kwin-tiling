kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS  = $(all_includes)
#
#noinst_LTLIBRARIES = libthemepage.la
#libthemepage_la_SOURCES = themepage.cpp previewwidget.cpp
#METASOURCES = AUTO 
#noinst_HEADERS = themepage.h previewwidget.h
#
