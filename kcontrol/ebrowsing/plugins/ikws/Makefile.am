## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/ikws

INCLUDES  =  -I$(srcdir)/../../kurifilter $(all_includes)
LDFLAGS   = $(KDE_RPATH) $(all_libraries)

lib_LTLIBRARIES = libkuriikwsfilter.la

libkuriikwsfilter_la_LDFLAGS = -version-info 1:0:0 -module
libkuriikwsfilter_la_LIBADD = ../../kurifilter/libkurifilter.la -lkdeui

libkuriikwsfilter_la_SOURCES = kuriikwsfiltereng.cpp kuriikwsfilter.skel kuriikwsfilter.cpp

service_DATA = kuriikwsfilter.desktop
servicedir = $(kde_servicesdir)

# rc_DATA = kuriikwsfilterrc
rcdir = $(kde_confdir)

METASOURCES = AUTO

messages:
	$(XGETTEXT) -C  -ktranslate -kaliasLocal -ki18n -x $(includedir)/kde.pot *.h *.cc && mv messages.po $(podir)/kuriikwsfilter.pot

