SUBDIRS= . sounds

kde_module_LTLIBRARIES = kcm_knotify.la

kcm_knotify_la_SOURCES = knotify.cpp

kcm_knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_knotify_la_LIBADD = $(LIB_KIO)
INCLUDES= $(all_includes)
 
kcm_knotify_la_METASOURCES = AUTO

noinst_HEADERS = knotify.h

messages:
	$(XGETTEXT) $(kcm_knotify_la_SOURCES) -o $(podir)/kcmnotify.pot

data_DATA = kcmnotify.desktop
datadir = $(kde_appsdir)/Settings/Sound

pics_DATA = play.png
picsdir = $(kde_datadir)/kcontrol/pics

# the following should be gone at release time.
install-data-local:
	rm -f $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/knotify.desktop
	rm -f $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/kcmnotify.desktop

