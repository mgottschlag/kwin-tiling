
kde_module_LTLIBRARIES        = kcm_launch.la
INCLUDES                      = $(all_includes) 
kcm_launch_la_SOURCES      = kcmlaunch.cpp
kcm_launch_la_LDFLAGS      = $(all_libraries) $(KDE_PLUGIN) -module
kcm_launch_la_LIBADD       = $(LIB_KDEUI)
kcm_launch_la_METASOURCES  = AUTO
noinst_HEADERS                = kcmlaunch.h
kcmlaunch_data_DATA                     = kcmlaunch.desktop
kcmlaunch_datadir                       = $(kde_appsdir)/Settings/LookNFeel/
EXTRA_DIST                    = $(kcmlaunch_data_DATA)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmlaunch.pot

