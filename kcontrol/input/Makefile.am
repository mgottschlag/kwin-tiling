SUBDIRS= pics

kde_module_LTLIBRARIES = kcm_input.la

kcm_input_la_SOURCES = main.cpp mouse.cpp kmousedlg.ui

kcm_input_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_input_la_LIBADD = $(XTESTLIB) $(LIB_KIO) $(LIB_KDEUI)

INCLUDES= $(all_includes)

kcm_input_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcminput.pot

data_DATA = mouse.desktop
datadir = $(kde_appsdir)/Settings/Peripherals

fontdir = $(kde_datadir)/kcminput
font_DATA = cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz
EXTRA_DIST = $(font_DATA)

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(prefix)/share/fonts/override
	-mkfontdir $(DESTDIR)$(prefix)/share/fonts/override
