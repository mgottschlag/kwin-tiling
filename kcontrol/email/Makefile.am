# $Id$

kde_module_LTLIBRARIES = kcm_email.la

kcm_email_la_SOURCES = emailbase.ui email.cpp
kcm_email_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_email_la_LIBADD = -lkdeui $(LIB_KIO)
INCLUDES= $(all_includes) 

kcm_email_la_METASOURCES = AUTO

noinst_HEADERS = email.h

KDE_ICON = email

data_DATA = email.desktop
datadir = $(kde_appsdir)/Settings

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmemail.pot
