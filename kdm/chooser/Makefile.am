# $Id$

AM_CXXFLAGS = $(STD_DEFINES) $(CONN_FLAGS)

INCLUDES = -I$(srcdir)/.. $(all_includes)

bin_PROGRAMS = chooser

chooser_SOURCES = \
    chooser.cpp \
    CXdmcp.cpp \
    CXdmcp_c.c \
    DXdmcp.cpp \
    kfdialog.cpp

chooser_LDFLAGS = $(all_libraries) $(KDE_RPATH)
chooser_LDADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(LIB_LIBS) \
    $(LIBSOCKET) $(LIBUCB) $(LIBUTIL)  -lXt

noinst_HEADERS = CXdmcp_c.h CXdmcp.h DXdmcp.h kfdialog.h

METASOURCES = CXdmcp.moc DXdmcp.moc kfdialog.moc

messages:
	$(XGETTEXT) $(chooser_SOURCES) -o $(podir)/kdmchooser.pot

EXTRA_DIST = Chooser.ad
