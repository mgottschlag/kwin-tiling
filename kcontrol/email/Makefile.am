# $Id$

kde_module_LTLIBRARIES = libkcm_email.la

libkcm_email_la_SOURCES = emailbase.ui email.cpp usernamedlg.cpp
libkcm_email_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_email_la_LIBADD = -lkdeui -lkfile 
INCLUDES= $(all_includes) 

libkcm_email_la_METASOURCES = AUTO

noinst_HEADERS = email.h usernamedlg.h 

KDE_ICON = email

data_DATA = email.desktop
datadir = $(kde_appsdir)/Settings/Personalization

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmemail.pot
