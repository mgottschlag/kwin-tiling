INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/lib

lib_LTLIBRARIES = libkickermain.la

libkickermain_la_SOURCES = abstractpanelmanager.cpp \
                           appletinfo.cpp utils.cpp kickertip.cpp \
                           menuinfo.cpp panelbutton.cpp panner.cpp \
                           kickerSettings.kcfgc kshadowsettings.cpp \
                           kshadowengine.cpp \
                           kpanelapplet.cpp  kpanelappmenu.cpp  \
                           kpanelextension.cpp  kpanelmenu.cpp \
                           simplebutton.cpp extensionSettings.kcfgc \
			   extensioninfo.cpp paneldrag.cpp

libkickermain_la_METASOURCES = AUTO

libkickermain_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libkickermain_la_LIBADD =  -L$(top_builddir)/workspace/lib -lkworkspace $(LIB_KIO)

libkickermain_la_includedir = $(includedir)/plasma

noinst_HEADERS = abstractpanelmanager.h appletinfo.h kickertip.h kpanelappmenu.h \
                 kshadowsettings.h panelbutton.h panner.h \
                 kshadowengine.h menuinfo.h \
		 extensioninfo.h
		 
libkickermain_la_include_HEADERS = kpanelapplet.h \
                                   kpanelextension.h \
                                   kpanelmenu.h \
                                   kickertip.h \
                                   utils.h \
				   paneldrag.h

kde_kcfg_DATA = kickerSettings.kcfg extensionSettings.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkicker.pot
