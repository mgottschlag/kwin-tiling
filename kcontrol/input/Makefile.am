if have_xcursor
THEMEPAGE_SUBDIR = xcursor
THEMEPAGE_INCLUDES = -I$(srcdir)/xcursor
THEMEPAGE_LIB = xcursor/libthemepage.la
else
THEMEPAGE_SUBDIR = core
THEMEPAGE_INCLUDES = -I$(srcdir)/core
THEMEPAGE_LIB = core/libthemepage.la
endif

SUBDIRS = $(THEMEPAGE_SUBDIR) pics

INCLUDES = $(THEMEPAGE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_input.la

kcm_input_la_SOURCES = mouse.cpp kmousedlg.ui main.cpp logitechmouse.cpp logitechmouse_base.ui
kcm_input_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_input_la_LIBADD = $(THEMEPAGE_LIB) $(XTESTLIB) $(LIB_KIO) $(LIB_XCURSOR) $(USB_LIBS)

kcm_input_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp */*.cpp -o $(podir)/kcminput.pot

xdg_apps_DATA = mouse.desktop

fontdir = $(kde_datadir)/kcminput
font_DATA = cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz
EXTRA_DIST = $(font_DATA)

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/fonts/override
	-mkfontdir $(DESTDIR)$(datadir)/fonts/override
##I'd like to install this automatically, but it only works on Linux, and I don't want to break things...
## $(INSTALL_DATA) $(srcdir)/consoleUserPerms $(DESTDIR)/etc/hotplug/usb/consoleUserPerms
## $(INSTALL_DATA) $(srcdir)/logitechmouse.usermap $(DESTDIR)/etc/hotplug/usb/logitechmouse.usermap
