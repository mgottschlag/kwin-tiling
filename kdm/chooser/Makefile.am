# $Id$

AM_CXXFLAGS = $(XDM_CFLAGS)
AM_CFLAGS = $(XDM_CFLAGS)

INCLUDES = -I$(srcdir)/../backend $(all_includes)

bin_PROGRAMS = chooser

chooser_SOURCES = \
    chooser.cpp \
    CXdmcp.cpp \
    CXdmcp_c.c \
    DXdmcp.cpp

chooser_LDFLAGS = $(all_libraries) $(KDE_RPATH)
chooser_LDADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(LIB_LIBS) \
    $(LIBSOCKET) $(LIBUCB) $(LIBUTIL)  -lXt

noinst_HEADERS = CXdmcp_c.h CXdmcp.h DXdmcp.h

METASOURCES = AUTO

messages:
	$(XGETTEXT) $(chooser_SOURCES) -o $(podir)/kdmchooser.pot
