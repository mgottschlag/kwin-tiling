AM_CPPFLAGS  = $(all_includes)

lib_LTLIBRARIES = 
kde_module_LTLIBRARIES = kcm_randr.la

noinst_LTLIBRARIES = librandrinternal.la

librandrinternal_la_SOURCES = ktimerdialog.cpp randr.cpp
METASOURCES = AUTO 

kcm_randr_la_SOURCES = krandrmodule.cpp
kcm_randr_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_randr_la_LIBADD = librandrinternal.la $(LIB_KDEUI) $(LIB_XRANDR)

noinst_HEADERS = randr.h krandrmodule.h krandrtray.h krandrapp.h ktimerdialog.h \
    krandrpassivepopup.h

xdg_apps_DATA = krandrtray.desktop

krandr_data_DATA = randr.desktop
krandr_datadir = $(kde_appsdir)/.hidden

KDE_ICON=AUTO

#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/Desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop 
#	$(DESTDIR)$(kde_appsdir)/Settings/Desktop/krandrmodule.desktop

bin_PROGRAMS = krandrtray

krandrtray_SOURCES = main.cpp krandrtray.cpp krandrapp.cpp krandrpassivepopup.cpp
krandrtray_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krandrtray_LDADD = librandrinternal.la $(LIB_KFILE) $(LIB_KUTILS) $(LIB_XRANDR)

