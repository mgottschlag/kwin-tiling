
INCLUDES = $(all_includes) 
LDFLAGS =  $(all_libraries) $(KDE_RPATH)
LDADD =    $(LIB_KDEUI) 

bin_PROGRAMS = kcmkdesktop
METASOURCES = rootopts.moc
noinst_HEADERS = rootopts.h

kcmkdesktop_SOURCES = main.cpp rootopts.cpp

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkdesktop_SOURCES) && mv messages.po ../../po/kcmkdesktop.pot

