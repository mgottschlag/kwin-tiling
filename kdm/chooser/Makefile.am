# $Id$

AM_CPPFLAGS = -DQT_NO_CAST_ASCII

INCLUDES = -I.. $(all_includes)

#AM_CFLAGS   = -Wall -Wno-implicit -Wno-return-type

bin_PROGRAMS = chooser

# Libraries:
#LDFLAGS = $(all_libraries) $(KDE_RPATH)
# $(KRB_RPATH)
#LDADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) \
#	$(LIB_LIBS) $(LIBSOCKET) \
#	$(LIBCRYPT) $(LIBUCB) $(LIBUTIL) 

chooser_SOURCES = \
	chooser.cpp \
	CXdmcp.cpp \
	DXdmcp.cpp \
	kfdialog.cpp

chooser_LDFLAGS = $(all_libraries) $(KDE_RPATH)
chooser_LDADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(LIB_LIBS) \
	$(LIBSOCKET) $(LIBUCB) $(LIBUTIL)  -lXt
#$(LIBCRYPT) 

noinst_HEADERS = CXdmcp.h DXdmcp.h kfdialog.h

METASOURCES = CXdmcp.moc DXdmcp.moc kfdialog.moc

messages:
	$(XGETTEXT) $(chooser_SOURCES) -o $(podir)/kdmchooser.pot

EXTRA_DIST = Chooser.ad
