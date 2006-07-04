



########### next target ###############

set(kickermain_LIB_SRCS 
   abstractpanelmanager.cpp 
   appletinfo.cpp 
   utils.cpp 
   kickertip.cpp 
   menuinfo.cpp 
   panelbutton.cpp 
   panner.cpp 
   kshadowsettings.cpp 
   kshadowengine.cpp 
   kpanelapplet.cpp 
   kpanelextension.cpp 
   kpanelmenu.cpp 
   simplebutton.cpp 
   extensioninfo.cpp 
   paneldrag.cpp )

kde4_automoc(${kickermain_LIB_SRCS})

kde4_add_kcfg_files(kickermain_LIB_SRCS kickerSettings.kcfgc extensionSettings.kcfgc )

kde4_add_library(kickermain SHARED ${kickermain_LIB_SRCS})

target_link_libraries(kickermain  ${KDE4_KDECORE_LIBS} kworkspace kio ${QT_QT3SUPPORT_LIBRARY} kdeui)

set_target_properties(kickermain PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kickermain  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES kickerSettings.kcfg extensionSettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install_files( /include/plasma FILES kpanelapplet.h                                     kpanelextension.h                                     kpanelmenu.h                                     kickertip.h                                     utils.h  				   paneldrag.h )




#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/lib
#
#lib_LTLIBRARIES = libkickermain.la
#
#libkickermain_la_SOURCES = abstractpanelmanager.cpp \
#                           appletinfo.cpp utils.cpp kickertip.cpp \
#                           menuinfo.cpp panelbutton.cpp panner.cpp \
#                           kickerSettings.kcfgc kshadowsettings.cpp \
#                           kshadowengine.cpp \
#                           kpanelapplet.cpp  \
#                           kpanelextension.cpp  kpanelmenu.cpp \
#                           simplebutton.cpp extensionSettings.kcfgc \
#			   extensioninfo.cpp paneldrag.cpp
#
#libkickermain_la_METASOURCES = AUTO
#
#libkickermain_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libkickermain_la_LIBADD =  -L$(top_builddir)/workspace/lib -lkworkspace $(LIB_KIO)
#
#libkickermain_la_includedir = $(includedir)/plasma
#
#noinst_HEADERS = abstractpanelmanager.h appletinfo.h kickertip.h \
#                 kshadowsettings.h panelbutton.h panner.h \
#                 kshadowengine.h menuinfo.h \
#		 extensioninfo.h
#		 
#libkickermain_la_include_HEADERS = kpanelapplet.h \
#                                   kpanelextension.h \
#                                   kpanelmenu.h \
#                                   kickertip.h \
#                                   utils.h \
#				   paneldrag.h
#
#kde_kcfg_DATA = kickerSettings.kcfg extensionSettings.kcfg
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libkicker.pot
