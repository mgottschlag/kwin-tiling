kcmSUBDIRS = docs

kde_module_LTLIBRARIES = kcm_fontinst.la
kcm_fontinst_la_LIBADD  = $(FREETYPE_LIB) $(LIB_KFILE)
kcm_fontinst_la_METASOURCES = AUTO
kcm_fontinst_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
 
desktop_DATA = kcmfontinst.desktop
desktopdir = $(kde_appsdir)/Settings/System/
 
kcm_fontinst_la_SOURCES = \
        AfmCreator.cpp BufferedFile.cpp CompressedFile.cpp Config.cpp \
        DirSettingsWidget.cpp DiskFontListWidget.cpp DisplaySettingsWidget.cpp \
        Encodings.cpp ErrorDialog.cpp FontEngine.cpp FontListWidget.cpp \
        FontPreview.cpp FontmapCreator.cpp FontsWidget.cpp \
        InstUninstSettingsWidget.cpp InstalledFontListWidget.cpp \
        Kfi.cpp KfiCmModule.cpp KfiGlobal.cpp KfiMainWidget.cpp \
        Misc.cpp SettingsWidget.cpp SettingsWizard.cpp StarOfficeConfig.cpp \
        StarOfficeSettingsWidget.cpp SysCfgSettingsWidget.cpp \
        SysConfigurer.cpp Ttf.cpp XConfig.cpp XftConfig.cpp \
        XftConfigEditor.cpp XftConfigIncludesWidget.cpp XftConfigRules.cpp \
        XftConfigSettingsWidget.cpp \
        DirSettingsWidgetData.ui DisplaySettingsWidgetData.ui \
        ErrorDialogData.ui FontListWidgetData.ui FontsWidgetData.ui \
        InstUninstSettingsWidgetData.ui KfiMainWidgetData.ui \
        SettingsWizardData.ui StarOfficeSettingsWidgetData.ui \
        SysCfgSettingsWidgetData.ui XftConfigEditorData.ui \
        XftConfigIncludesWidgetData.ui XftConfigRulesData.ui \
        XftConfigSettingsWidgetData.ui \
	xftgram-kfi.c \
	xftlex-kfi.c

xftparserfiles = xftlex.l xftgram.y

EXTRA_DIST = \
	$(desktop_DATA) \
	$(kcm_fontinst_la_SOURCES) \
	$(xftparserfiles) \
	KfiCmModule.h \
	Config.h \
	DiskFontListWidget.h \
	Encodings.h \
	FontEngine.h \
	FontListWidget.h\
	FontsWidget.h \
	InstalledFontListWidget.h \
	Kfi.h \
	KfiGlobal.h \
	KfiMainWidget.h \
	Misc.h \
	SettingsWidget.h \
	DirSettingsWidget.h \
	DisplaySettingsWidget.h \
	InstUninstSettingsWidget.h \
	StarOfficeSettingsWidget.h \
	SysCfgSettingsWidget.h \
	ErrorDialog.h \
	Ttf.h \
	AfmCreator.h \
	SysConfigurer.h \
	FontmapCreator.h \
	BufferedFile.h \
	StarOfficeConfig.h \
	XConfig.h \
	SettingsWizard.h \
	XftConfigSettingsWidget.h \
	XftConfig.h \
	XftConfigRules.h \
	XftConfigEditor.h \
	XftRename.h \
	FontPreview.h \
	CompressedFile.h \
	xftint.h \
	xftgram.h \
	xftgram.c \
	xftlex.c \
	hi32-app-kcmfontinst.png \
	hi48-app-kcmfontinst.png \
	lo16-app-kcmfontinst.png \
	lo32-app-kcmfontinst.png \
	kcmfontinst_star_office.png

noinst_HEADERS = $(EXTRA_DIST)

INCLUDES= $(FREETYPE_INC) $(all_includes)
KDE_ICON = kcmfontinst

pics_DATA = kcmfontinst_star_office.png
picsdir = $(kde_datadir)/kcmfontinst/pics

xftparser: $(xftparserfiles)
	cd $(srcdir); \
	flex -PKfiXftConfig -oxftlex.c xftlex.l ;\
	bison -pKfiXftConfig -t -d -o xftgram.c xftgram.y

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/fontinst.pot
