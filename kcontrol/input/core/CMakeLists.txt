kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS  = $(all_includes)
#
#noinst_LTLIBRARIES = libthemepage.la
#libthemepage_la_SOURCES = themepage.cpp
#METASOURCES = AUTO 
#noinst_HEADERS = themepage.h bitmaps.h
#
