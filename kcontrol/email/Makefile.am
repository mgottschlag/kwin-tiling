lib_LTLIBRARIES = libkcm_email.la

libkcm_email_la_SOURCES = email.cpp 

libkcm_email_la_LDFLAGS  = $(all_libraries) -module -avoid-version
libkcm_email_la_LIBADD = -lkdeui  
INCLUDES= $(all_includes) 

libkcm_email_la_METASOURCES = AUTO

noinst_HEADERS = email.h

KDE_ICON = email

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(libkcm_email_la_SOURCES) && mv messages.po ../../po/kcmemail.pot

