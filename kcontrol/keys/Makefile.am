kde_module_LTLIBRARIES = libkcm_keys.la

libkcm_keys_la_SOURCES =  shortcuts.cpp modifiers.cpp savescm.cpp main.cpp
libkcm_keys_la_LIBADD   = -lkdeui
libkcm_keys_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined

INCLUDES= $(all_includes)

libkcm_keys_la_METASOURCES = AUTO

messages:
	customkeys=`grep "^.include .\.\." keyconfig.cpp | sed -e "s#.*\"\(.*\)\"#\1#"` ;\
	$(XGETTEXT) $(libkcm_keys_la_SOURCES) $$customkeys -o $(podir)/kcmkeys.pot

data_DATA = keys.desktop
datadir = $(kde_appsdir)/Settings/LookNFeel

scheme_DATA = kde3.kksrc kde4.kksrc win3.kksrc win4.kksrc mac4.kksrc unix3.kksrc
schemedir = $(kde_datadir)/kcmkeys
