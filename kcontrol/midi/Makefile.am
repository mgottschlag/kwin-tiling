kde_module_LTLIBRARIES = kcm_midi.la

kcm_midi_la_SOURCES = midi.cpp 

kcm_midi_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_midi_la_LIBADD = -lkdeui $(LIB_KFILE) -lkmid 
INCLUDES= $(all_includes) 

kcm_midi_la_METASOURCES = AUTO

noinst_HEADERS = midi.h

KDE_ICON = midi

messages:
	$(XGETTEXT) $(kcm_midi_la_SOURCES) -o $(podir)/kcmmidi.pot

data_DATA = midi.desktop
datadir = $(kde_appsdir)/Settings/Sound
