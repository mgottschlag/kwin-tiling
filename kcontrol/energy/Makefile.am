# $Id$

SUBDIRS= . pics

INCLUDES = $(all_includes) $(DPMSINC)

kde_module_LTLIBRARIES = kcm_energy.la

kcm_energy_la_SOURCES = energy.cpp
kcm_energy_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_energy_la_LIBADD = $(DPMSLIB) $(LIB_KDEUI) 
kcm_energy_la_METASOURCES = energy.moc

noinst_HEADERS = energy.h

messages:
	$(XGETTEXT) -kni18n $(kcm_energy_la_SOURCES) -o $(podir)/kcmenergy.pot

power_DATA = energy.desktop
powerdir = $(kde_appsdir)/Settings/PowerControl

EXTRA_DIST = $(power_DATA)
