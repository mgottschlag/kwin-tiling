
INCLUDES	= $(all_includes)
LDFLAGS		= $(all_libraries) $(KDE_RPATH)
LDADD		= $(LIB_KDEUI) $(LIB_KIMGIO) $(LIB_KFILE) $(LIB_KIO)
SUBDIRS		= pics drvcfg1 drvcfg2

noinst_HEADERS = 		KODBCConfig.h \
				classAbout.h \
				classDSNList.h \
				classDLL.cpp \
				classDriverPrompt.h \
				classDrivers.h \
				classTracing.h \
		       		classFileSelector.h \
				classProperties.h \
				classSystemDSN.h \
				classUserDSN.h

lib_LTLIBRARIES =		libkcm_odbc.la

libkcm_odbc_la_SOURCES = 	KODBCConfig.cpp \
				classAbout.cpp \
				classDLL.cpp \
		       		classDSNList.cpp \
				classDriverPrompt.cpp \
				classDrivers.cpp \
				classTracing.cpp \
				classFileSelector.cpp \
				classProperties.cpp \
				classSystemDSN.cpp \
				classUserDSN.cpp

libkcm_odbc_la_LDFLAGS = 	-module -avoid-version $(all_libraries) 


libkcm_odbc_la_LIBADD = 	$(LIB_KDEUI) $(LIB_KIMGIO) $(LIB_KFILE) $(DPMSLIB) \
				$(LDADD) -lodbcinst -luodbcini

METASOURCES =			AUTO

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot \
	$(libkcm_odbc_la_SOURCES) && mv messages.po $(podir)/kcmodbc.pot

