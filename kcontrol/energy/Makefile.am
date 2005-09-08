SUBDIRS= . pics

AM_CPPFLAGS = $(all_includes) $(DPMSINC)

kde_module_LTLIBRARIES = kcm_energy.la

kcm_energy_la_SOURCES = energy.cpp
kcm_energy_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_energy_la_LIBADD = $(DPMSLIB) $(LIB_KDEUI) $(LIB_KIO)
METASOURCES = energy.moc

noinst_HEADERS = energy.h

messages:
	$(XGETTEXT) -kni18n $(kcm_energy_la_SOURCES) -o $(podir)/kcmenergy.pot

energy_data_DATA = energy.desktop
energy_datadir = $(kde_appsdir)/.hidden
KDE_ICON=AUTO
