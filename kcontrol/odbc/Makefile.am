
INCLUDES	= $(all_includes)
LDFLAGS		= $(all_libraries) $(KDE_RPATH)
LDADD		= $(LIB_KIMGIO) $(LIB_KFILE) $(LIB_KIO)
SUBDIRS		= pics drvcfg1 drvcfg2

bin_PROGRAMS = odbcconfig

odbcconfig_SOURCES =	main.cpp \
			classAbout.cpp \
			classDLL.cpp \
			classDSNList.cpp \
			classDriverPrompt.cpp \
			classDrivers.cpp \
			classFileSelector.cpp \
			classProperties.cpp \
			classSystemDSN.cpp \
			classUserDSN.cpp

odbcconfig_LDADD = $(LDADD) -lodbcinst -luodbcini

METASOURCES =		classAbout.moc \
			classDSNList.moc \
			classDriverPrompt.moc \
			classDrivers.moc \
			classFileSelector.moc \
			classProperties.moc \
			classSystemDSN.moc \
			classUserDSN.moc

noinst_HEADERS = 	classAbout.h \
			classDSNList.h \
			classDLL.cpp \
			classDriverPrompt.h \
			classDrivers.h \
			classFileSelector.h \
			classProperties.h \
			classSystemDSN.h \
			classUserDSN.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(odbcconfig_SOURCES) && mv messages.po $(podir)/odbcconfig.pot

