kde_module_LTLIBRARIES = kcm_arts.la

kcm_arts_la_SOURCES = arts.cpp midi.cpp arts_general.ui arts_soundio.ui

kcm_arts_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_arts_la_LIBADD = -lkio -lkmid

INCLUDES = -I$(kde_includes)/arts  $(all_includes)

kcm_arts_la_METASOURCES = AUTO

KDE_ICON = arts

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmarts.pot

kcmarts_data_DATA = arts.desktop
kcmarts_datadir = $(kde_appsdir)/Settings/Sound
kcmarts_data2_DATA = midi.desktop
kcmarts_data2dir = $(kde_appsdir)/.hidden

