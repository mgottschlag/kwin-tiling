
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =    $(LIB_KDEUI) $(LIB_KFILE)

bin_PROGRAMS = kcmroot

kcmroot_SOURCES = main.cpp 
METASOURCES = main.moc 
noinst_HEADERS = main.h

KDE_ICON = root

messages:
#	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmroot_SOURCES) && mv messages.po ../../po/kcmroot.pot

