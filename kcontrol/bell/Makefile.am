
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =    $(LIB_KDEUI) 

bin_PROGRAMS = kcmbell

kcmbell_SOURCES = main.cpp bell.cpp 
METASOURCES = bell.moc 
noinst_HEADERS = bell.h geom.h 

KDE_ICON = bell

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmbell_SOURCES) && mv messages.po ../../po/kcmbell.pot

