INCLUDES= $(all_includes)

####### Files

bin_PROGRAMS = 	kthememgr
kthememgr_SOURCES = main.cpp theme.cpp installer.cpp kwmcom.cpp \
	options.cpp widgetcanvas.cpp about.cpp groupdetails.cpp \
	themecreator.cpp newthemedlg.cpp
noinst_HEADERS = theme.h installer.h kwmcom.h widgetcanvas.h \
	options.h colors.h about.h version.h groupdetails.h \
	themecreator.h newthemedlg.h

kthememgr_LDADD = $(LIB_KIMGIO) $(LIB_KFILE) 
kthememgr_LDFLAGS = $(all_libraries) $(LDFLAGS) $(KDE_RPATH)

METASOURCES = theme.moc installer.moc colors.moc widgetcanvas.moc \
	options.moc about.moc groupdetails.moc 

KDE_ICON = kthememgr

apps_DATA = kthememgr.desktop
appsdir = $(kde_appsdir)/Settings/Desktop 

mappingdir = $(kde_datadir)/kthememgr
mapping_DATA = theme.mappings

themesdir =  $(kde_datadir)/kthememgr/Themes
themes_DATA = Template.themerc

install-themes:
	cd $(srcdir)/Themes; \
	for f in *; do \
	  if [ -d $$f -a $$f != "CVS" ]; then \
	    tar --exclude CVS -c -f - $$f | gzip -c > $(DESTDIR)$(themesdir)/$$f.tar.gz; \
	  fi; \
	done

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kthememgr_SOURCES) && mv messages.po $(podir)/kthememgr.pot

edit:
	xemacs $(kthememgr_SOURCES) $(noinst_HEADERS) &

