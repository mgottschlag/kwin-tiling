kde_module_LTLIBRARIES = kcm_keys.la

kcm_keys_la_SOURCES =  shortcuts.cpp modifiers.cpp main.cpp khotkeys.cpp treeview.cpp commandShortcuts.cpp
kcm_keys_la_LIBADD   = -lkdeui -lkio
kcm_keys_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined

AM_CPPFLAGS= $(all_includes)

METASOURCES = AUTO

messages:
	customkeys=`grep "^.include .\.\." keyconfig.cpp | sed -e "s#.*\"\(.*\)\"#\1#"` ;\
	$(XGETTEXT) $(kcm_keys_la_SOURCES) $$customkeys -o $(podir)/kcmkeys.pot

update_DATA = kaccel.upd
update_SCRIPTS = convertShortcuts.pl
updatedir = $(kde_datadir)/kconf_update

xdg_apps_DATA = keys.desktop

scheme_DATA = wm3.kksrc kde3.kksrc kde4.kksrc win3.kksrc win4.kksrc mac4.kksrc unix3.kksrc
schemedir = $(kde_datadir)/kcmkeys
KDE_ICON=AUTO
