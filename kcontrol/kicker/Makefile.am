kde_module_LTLIBRARIES = libkcm_kicker.la

libkcm_kicker_la_SOURCES = generaltab.ui lookandfeeltab.ui menutab.ui extensionstab.ui \
			generaltab_impl.cpp lookandfeeltab_impl.cpp menutab_impl.cpp \
			buttontab.cpp applettab.cpp main.cpp

libkcm_kicker_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_kicker_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KFILE)
INCLUDES= $(all_includes) 

libkcm_kicker_la_METASOURCES = AUTO

noinst_HEADERS = generaltab_impl.h lookandfeeltab_impl.h buttontab.h menutab_impl.h applettab.h main.h

KDE_ICON = go

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(libkcm_kicker_la_SOURCES) -o $(podir)/kcmkicker.pot

data_DATA = panel.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel/

EXTRA_DIST = $(data_DATA)
