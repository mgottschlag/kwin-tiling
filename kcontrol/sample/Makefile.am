
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

noinst_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp

KDE_ICON = sample

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsample_SOURCES) && mv messages.po $(podir)/kcmsample.pot

