
INCLUDES = $(all_includes) 
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KDEUI) 

bin_PROGRAMS = kcmsamba

kcmsamba_SOURCES = main.cpp ksmbstatus.cpp
METASOURCES = ksmbstatus.moc
noinst_HEADERS = ksmbstatus.h

KDE_ICON = smbstatus

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsamba_SOURCES) && mv messages.po ../../po/kcmsamba.pot

