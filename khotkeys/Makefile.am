SUBDIRS = shared kcontrol app data update

khotkeys_autostart_DATA = khotkeys.desktop
khotkeys_autostartdir = $(prefix)/share/autostart

EXTRA_DIST = $(khotkeys_autostart_DATA) COPYING AUTHORS

messages: rc.cpp
	$(EXTRACTRC) kcontrol/ui/*.ui >> rc.cpp
	$(XGETTEXT) rc.cpp app/*.cpp shared/*.cpp kcontrol/*.cpp -o $(podir)/khotkeys.pot
