kde_module_LTLIBRARIES = kcm_kicker.la

kcm_kicker_la_SOURCES = positiontab.ui hidingtab.ui lookandfeeltab.ui menutab.ui \
			positiontab_impl.cpp hidingtab_impl.cpp lookandfeeltab_impl.cpp \
			menutab_impl.cpp extensionInfo.cpp main.cpp menutab_kcm.cpp lookandfeeltab_kcm.cpp

kcm_kicker_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_kicker_la_LIBADD =  ../background/libbgnd.la $(LIB_KIO)
INCLUDES= -I$(srcdir)/../background $(all_includes)

kcm_kicker_la_METASOURCES = AUTO

noinst_HEADERS = positiontab_impl.h hidingtab_impl.h lookandfeeltab_impl.h menutab_impl.h \
		extensionInfo.h main.h lookandfeeltab_kcm.h menutab_kcm.h

KDE_ICON = go

# Translation of tiles is used by lookandfeeltab_impl.cpp
# Tile names are transformed to words with title case
messages: rc.cpp
	(cd ../../kicker/data/tiles ; ls *_tiny_up.png) | perl -p -e \
	  's/(.*)_tiny_up\.png/i18n\(\"\u$$1\"\)\;/; s/[_ ]+(.)/ \u$$1/g' >> rc.cpp
	$(XGETTEXT) rc.cpp $(kcm_kicker_la_SOURCES) -o $(podir)/kcmkicker.pot

data_DATA = panel.desktop panel_appearance.desktop panel_menus.desktop
datadir = $(kde_appsdir)/Settings/Panel/

EXTRA_DIST = $(data_DATA)
