# $Id$

SUBDIRS= . pics

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkcm_energy.la

libkcm_energy_la_SOURCES = energy.cpp
libkcm_energy_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_energy_la_LIBADD = $(LIB_KDEUI) $(DPMSLIB)
libkcm_energy_la_METASOURCES = energy.moc

noinst_HEADERS = energy.h

messages:
	$(XGETTEXT) -kni18n $(libkcm_energy_la_SOURCES) -o $(podir)/kcmenergy.pot

power_DATA = energy.desktop
powerdir = $(kde_appsdir)/Settings/PowerControl

EXTRA_DIST = $(power_DATA)
