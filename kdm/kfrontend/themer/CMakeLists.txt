kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -I$(srcdir)/../../backend -I$(srcdir)/.. -I../.. \
#	-I$(top_srcdir)/kdmlib \
#	$(all_includes)
#
#noinst_LIBRARIES = libkdmthemer.a
#libkdmthemer_a_SOURCES = \
#    kdmthemer.cpp \
#    kdmitem.cpp \
#    kdmpixmap.cpp \
#    kdmrect.cpp \
#    kdmlabel.cpp \
#    kdmlayout.cpp
#
#METASOURCES = AUTO
#
#libkdmthemer_a_COMPILE_FIRST = ../../config.ci
