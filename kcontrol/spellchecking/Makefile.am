kde_module_LTLIBRARIES = kcm_spellchecking.la

kcm_spellchecking_la_SOURCES = spellchecking.cpp

kcm_spellchecking_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_spellchecking_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

kcm_spellchecking_la_METASOURCES = spellchecking.moc

KDE_ICON = spellchecking

messages:
	$(XGETTEXT) $(kcm_spellchecking_la_SOURCES) -o $(podir)/kcmspellchecking.pot

kcmspellchecking_data_DATA = spellchecking.desktop
kcmspellchecking_datadir = $(kde_appsdir)/Settings/Components
