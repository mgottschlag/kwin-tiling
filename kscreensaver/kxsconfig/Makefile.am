## Makefile.am of kdebase/kscreensaver/kxsconfig

INCLUDES = $(all_includes)
kxsconfig_LDFLAGS  = $(KDE_RPATH) $(all_libraries) 
kxsconfig_LDADD    = $(LIB_KDEUI) $(LIB_KIMGIO)

kxsrun_LDFLAGS  = $(KDE_RPATH) $(all_libraries) 
kxsrun_LDADD    = $(LIB_KDECORE)

SUBDIRS = config

####### Files

bin_PROGRAMS = kxsconfig kxsrun

kxsconfig_SOURCES = kxsconfig.cpp kxscontrol.cpp kxsitem.cpp
kxsrun_SOURCES = kxsrun.cpp kxsitem.cpp

include_HEADERS = 

noinst_HEADERS = kxsconfig.h kxscontrol.h kxsitem.h

METASOURCES = AUTO

####### Build rules

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -ki18n -x $(includedir)/kde.pot *.cc *.h && mv messages.po ../po/kxsconfig.pot


