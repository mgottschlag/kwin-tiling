INCLUDES= $(all_includes)

####### Files

METASOURCES = AUTO

lib_LTLIBRARIES = libkcm_themes.la

libkcm_themes_la_SOURCES = main.cpp installer.cpp \
	options.cpp about.cpp groupdetails.cpp \
	themecreator.cpp newthemedlg.cpp theme.cpp
noinst_HEADERS = theme.h installer.h \
	options.h about.h version.h groupdetails.h \
	themecreator.h newthemedlg.h

libkcm_themes_la_LIBADD = $(LIB_KFILE) 
libkcm_themes_la_LDFLAGS = $(all_libraries) $(LDFLAGS) -module -avoid-version -no-undefined

apps_DATA = kthememgr.desktop
appsdir = $(kde_appsdir)/Settings/LookNFeel

mappingdir = $(kde_datadir)/kthememgr
mapping_DATA = theme.mappings

kdemime_DATA = x-ktheme.desktop
kdemimedir = $(kde_mimedir)/application

themesdir =  $(kde_datadir)/kthememgr/Themes
themes_DATA = Template.themerc

install-data-local:
	cd $(srcdir)/Themes; \
	for f in *; do \
	  if [ -d $$f -a $$f != "CVS" ]; then \
	    tar --exclude CVS -c -f - $$f | gzip -c > $(DESTDIR)$(themesdir)/$$f.ktheme; \
	  fi; \
	done

messages:
	$(XGETTEXT) $(libkcm_themes_la_SOURCES) -o $(podir)/kcmthemes.pot

