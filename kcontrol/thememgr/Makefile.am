INCLUDES= $(all_includes)

####### Files

METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_themes.la

kcm_themes_la_SOURCES = main.cpp installer.cpp \
	options.cpp groupdetails.cpp \
	themecreator.cpp newthemedlg.cpp theme.cpp
noinst_HEADERS = theme.h installer.h \
	options.h version.h groupdetails.h \
	themecreator.h newthemedlg.h

kcm_themes_la_LIBADD = $(LIB_KFILE) 
kcm_themes_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

apps_DATA = kthememgr.desktop
appsdir = $(kde_appsdir)/Settings/LookNFeel

mappingdir = $(kde_datadir)/kthememgr
mapping_DATA = theme.mappings

kdemime_DATA = x-ktheme.desktop
kdemimedir = $(kde_mimedir)/application

themesdir =  $(kde_datadir)/kthememgr/Themes
themes_DATA = Template.themerc

install-data-local:
	cp -r $(srcdir)/Themes BuildThemes; \
	rm -rf `find BuildThemes -type d -name CVS -print`; \
	mkdir -p $(DESTDIR)$(themesdir)
	( cd BuildThemes; \
	for f in *; do \
	  if [ -d $$f -a $$f != "CVS" ]; then \
           tar=$(TAR); \
           if test -z "$$tar"; then tar=$(AMTAR); fi; \
           if test -z "$$tar"; then tar=tar; fi; \
           $$tar -b 16 -c -f - $$f| gzip -c > $(DESTDIR)$(themesdir)/$$f.ktheme; \
	  fi; \
	done ); \
	rm -rf BuildThemes;

messages:
	(cd Themes && ls -1 | grep -v CVS | sed -e "s/\(.*\)/i18n(\"\1\");/") > rc.cpp
	$(XGETTEXT) $(kcm_themes_la_SOURCES) rc.cpp -o $(podir)/kcmthemes.pot
	-rm -f rc.cpp

