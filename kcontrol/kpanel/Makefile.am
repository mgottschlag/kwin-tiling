SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

bin_PROGRAMS = kcmkpanel

kcmkpanel_SOURCES = main.cpp desktops.cpp panel.cpp options.cpp disknav.cpp \
	widgetcanvas.cpp paneltheme.cpp
METASOURCES = panel.moc desktops.moc options.moc disknav.moc  \
	widgetcanvas.moc paneltheme.moc
noinst_HEADERS = panel.h desktops.h options.h disknav.h \
	widgetcanvas.h paneltheme.h

messages:
	$(XGETTEXT) $(kcmkpanel_SOURCES) -o $(podir)/kcmkpanel.pot

KDE_ICON = kcmkpanel

