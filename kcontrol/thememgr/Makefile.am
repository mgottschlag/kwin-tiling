INCLUDES= $(all_includes)

####### Files

METASOURCES = AUTO

lib_LTLIBRARIES = libkcm_themes.la

libkcm_themes_la_SOURCES = main.cpp theme.cpp installer.cpp kwmcom.cpp \
	options.cpp widgetcanvas.cpp about.cpp groupdetails.cpp \
	themecreator.cpp newthemedlg.cpp
noinst_HEADERS = theme.h installer.h kwmcom.h widgetcanvas.h \
	options.h colors.h about.h version.h groupdetails.h \
	themecreator.h newthemedlg.h

libkcm_themes_la_LIBADD = $(LIB_KFILE) 
libkcm_themes_la_LDFLAGS = $(all_libraries) $(LDFLAGS) -module -avoid-version -no-undefined

apps_DATA = kthememgr.desktop
appsdir = $(kde_appsdir)/Settings/LookNFeel

mappingdir = $(kde_datadir)/kthememgr
mapping_DATA = theme.mappings

themesdir =  $(kde_datadir)/kthememgr/Themes
themes_DATA = Template.themerc

install-data-local:
	cd $(srcdir)/Themes; \
	for f in *; do \
	  if [ -d $$f -a $$f != "CVS" ]; then \
	    tar --exclude CVS -c -f - $$f | gzip -c > $(DESTDIR)$(themesdir)/$$f.tar.gz; \
	  fi; \
	done

messages:
	$(XGETTEXT) $(libkcm_themes_la_SOURCES) -o $(podir)/kcmthemes.pot

