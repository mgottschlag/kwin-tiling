SUBDIRS = docs

lib_LTLIBRARIES = libkcm_kfontinst.la
libkcm_kfontinst_la_LIBADD  = $(FREETYPE_LIB) $(LIB_KFILE)
libkcm_kfontinst_la_METASOURCES = AUTO
libkcm_kfontinst_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH)
 
desktop_DATA = kfontinst.desktop
desktopdir = $(kde_appsdir)/Settings/System/
 
libkcm_kfontinst_la_SOURCES = \
	KfiCmModule.cpp \
	Config.cpp \
	DiskFontListWidget.cpp \
	Encodings.cpp \
	FontEngine.cpp \
	FontListWidget.cpp\
	FontListWidgetData.cpp \
	FontsWidget.cpp \
	FontsWidgetData.cpp \
	InstalledFontListWidget.cpp \
	Kfi.cpp \
	KfiGlobal.cpp \
	KfiMainWidget.cpp \
	KfiMainWidgetData.cpp \
	Misc.cpp \
	SettingsWidget.cpp \
	DirSettingsWidget.cpp \
	DirSettingsWidgetData.cpp \
	DisplaySettingsWidget.cpp \
	DisplaySettingsWidgetData.cpp \
	InstUninstSettingsWidget.cpp \
	InstUninstSettingsWidgetData.cpp \
	StarOfficeSettingsWidget.cpp \
	StarOfficeSettingsWidgetData.cpp \
	SysCfgSettingsWidget.cpp \
	SysCfgSettingsWidgetData.cpp \
	ErrorDialog.cpp \
	ErrorDialogData.cpp \
	Ttf.cpp \
	AfmCreator.cpp \
	SysConfigurer.cpp \
	FontmapCreator.cpp \
	BufferedFile.cpp \
	StarOfficeConfig.cpp \
	XConfig.cpp \
	AboutWidget.cpp \
	AboutWidgetData.cpp \
	SettingsWizard.cpp \
	SettingsWizardData.cpp \
	XftConfigSettingsWidget.cpp \
	XftConfigSettingsWidgetData.cpp \
	XftConfig.cpp \
	XftConfigRules.cpp \
	XftConfigRulesData.cpp \
	XftConfigEditor.cpp \
	XftConfigEditorData.cpp \
	XftConfigIncludesWidget.cpp \
	XftConfigIncludesWidgetData.cpp \
	xftgram-kfi.c \
	xftlex-kfi.c

xftparserfiles = xftlex.l xftgram.y

EXTRA_DIST = \
	$(desktop_DATA) \
	$(libkcm_kfontinst_la_SOURCES) \
	$(xftparserfiles) \
	KfiCmModule.h \
	Config.h \
	DiskFontListWidget.h \
	Encodings.h \
	FontEngine.h \
	FontListWidget.h\
	FontListWidgetData.h \
	FontsWidget.h \
	FontsWidgetData.h \
	InstalledFontListWidget.h \
	Kfi.h \
	KfiGlobal.h \
	KfiMainWidget.h \
	KfiMainWidgetData.h \
	Misc.h \
	SettingsWidget.h \
	DirSettingsWidget.h \
	DirSettingsWidgetData.h \
	DisplaySettingsWidget.h \
	DisplaySettingsWidgetData.h \
	InstUninstSettingsWidget.h \
	InstUninstSettingsWidgetData.h \
	StarOfficeSettingsWidget.h \
	StarOfficeSettingsWidgetData.h \
	SysCfgSettingsWidget.h \
	SysCfgSettingsWidgetData.h \
	ErrorDialog.h \
	ErrorDialogData.h \
	Ttf.h \
	AfmCreator.h \
	SysConfigurer.h \
	FontmapCreator.h \
	BufferedFile.h \
	StarOfficeConfig.h \
	XConfig.h \
	AboutWidget.h \
	AboutWidgetData.h \
	SettingsWizard.h \
	SettingsWizardData.h \
	XftConfigSettingsWidget.h \
	XftConfigSettingsWidgetData.h \
	XftConfig.h \
	XftConfigRules.h \
	XftConfigRulesData.h \
	XftConfigEditor.h \
	XftConfigEditorData.h \
	xftint.h \
	xftgram.h \
	xftgram.c \
	xftlex.c \
	DirSettingsWidget.ui \
	DisplaySettingsWidget.ui \
	FontListWidget.ui \
	FontsWidget.ui \
	InstUninstSettingsWidget.ui \
	KfiMainWidget.ui \
	StarOfficeSettingsWidget.ui \
	SysCfgSettingsWidget.ui \
	ErrorDialog.ui \
	SettingsWizard.ui \
	XftConfigSettingsWidget.ui \
	XftConfigRules.ui \
        XftConfigEditor.ui \
	XftConfigIncludesWidget.h \
	XftConfigIncludesWidgetData.h \
	XftConfigIncludesWidget.ui \
	hi32-app-kfontinst.png \
	hi48-app-kfontinst.png \
	lo16-app-kfontinst.png \
	lo32-app-kfontinst.png \
	so.png

noinst_HEADERS = $(EXTRA_DIST)

INCLUDES= $(FREETYPE_INC) $(all_includes)
KDE_ICON = kfontinst

pics_DATA = so.png
picsdir = $(kde_datadir)/kfontinst/pics

DISTCLEANFILES = $(kfontinst_METASOURCES)

xftparser: $(xftparserfiles)
	cd $(srcdir); \
	flex -PXftConfig -oxftlex.c xftlex.l ;\
	bison -pXftConfig -t -d -o xftgram.c xftgram.y

messages:
	$(XGETTEXT) -kni18n $(libkcm_kfontinst_la_SOURCES) -o $(podir)/kfontinst.pot
