kde_module_LTLIBRARIES = kcm_arts.la

kcm_arts_la_SOURCES = arts.cpp generaltab.ui hardwaretab.ui krichtextlabel.cpp

kcm_arts_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_arts_la_LIBADD = $(LIB_KIO) $(LIB_KUTILS) -lkmid

AM_CPPFLAGS = -I$(kde_includes)/arts  $(all_includes)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmarts.pot

xdg_apps_DATA = arts.desktop

KDE_ICON=AUTO
