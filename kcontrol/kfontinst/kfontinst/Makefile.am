kcmSUBDIRS = docs

kde_module_LTLIBRARIES = kcm_fontinst.la
kcm_fontinst_la_LIBADD  = $(FREETYPE_LIB) $(LIB_KFILE)
kcm_fontinst_la_METASOURCES = AUTO
kcm_fontinst_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
 
desktop_DATA = kcmfontinst.desktop
desktopdir = $(kde_appsdir)/Settings/System/
 
kcm_fontinst_la_SOURCES = \
        AfmCreator.cpp BufferedFile.cpp CompressedFile.cpp Config.cpp \
        Encodings.cpp ErrorDialog.cpp FontEngine.cpp FontListWidget.cpp \
        FontPreview.cpp FontmapCreator.cpp FontsWidget.cpp \
        Kfi.cpp KfiCmModule.cpp KfiGlobal.cpp KfiMainWidget.cpp \
        Misc.cpp SettingsWidget.cpp SettingsWizard.cpp StarOfficeConfig.cpp \
        SysConfigurer.cpp Ttf.cpp XConfig.cpp \
        ErrorDialogData.ui \
        KfiMainWidgetData.ui \
        SettingsWidgetData.ui SettingsWizardData.ui

EXTRA_DIST = \
	$(desktop_DATA) \
	$(kcm_fontinst_la_SOURCES) \
	KfiCmModule.h \
	Config.h \
	Encodings.h \
	FontEngine.h \
	FontListWidget.h\
	FontsWidget.h \
	Kfi.h \
	KfiGlobal.h \
	KfiMainWidget.h \
	Misc.h \
	SettingsWidget.h \
	ErrorDialog.h \
	Ttf.h \
	AfmCreator.h \
	SysConfigurer.h \
	FontmapCreator.h \
	BufferedFile.h \
	StarOfficeConfig.h \
	XConfig.h \
	SettingsWizard.h \
	FontPreview.h \
	CompressedFile.h \
	hi32-app-kcmfontinst.png \
	hi48-app-kcmfontinst.png \
	lo16-app-kcmfontinst.png \
	lo32-app-kcmfontinst.png \
	kcmfontinst_star_office.png

noinst_HEADERS = $(EXTRA_DIST)

INCLUDES= $(FREETYPE_INC) $(all_includes) -I$(srcdir)/../../fonts/
KDE_ICON = kcmfontinst

pics_DATA = kcmfontinst_star_office.png
picsdir = $(kde_datadir)/kcmfontinst/pics

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/fontinst.pot
