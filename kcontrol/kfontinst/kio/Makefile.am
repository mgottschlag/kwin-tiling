kde_module_LTLIBRARIES = kio_fonts.la
kio_fonts_la_SOURCES = \
KioFonts.cpp

kio_fonts_la_LIBADD = $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_KIO) -lkdesu ../lib/libkfontinst.la
kio_fonts_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -module -avoid-version -no-undefined
INCLUDES= $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) $(all_includes) -I$(srcdir)/../lib -I$(srcdir)/../../fonts

# The kxftconfig stuf really belongs to kdebase/kcontrol/fonts - here only so that can distribute this as an archive.
noinst_HEADERS = \
KioFonts.h

servicesdir = $(kde_servicesdir)
services_DATA = fonts.protocol

vfsdata_DATA = fonts.desktop
vfsdatadir = $(kde_datadir)/konqsidebartng/virtual_folders/services

mimetype_DATA = folder.desktop system-folder.desktop
mimetypedir = $(kde_mimedir)/fonts

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_fonts.pot
