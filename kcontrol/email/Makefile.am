
INCLUDES = $(all_includes) 
LDFLAGS =  $(all_libraries) $(KDE_RPATH)
LDADD =    $(LIB_KDEUI)

bin_PROGRAMS = kcmemail

kcmemail_SOURCES = main.cpp email.cpp
METASOURCES = AUTO
noinst_HEADERS = email.h

KDE_ICON = email

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmemail_SOURCES) && mv messages.po ../../po/kcmemail.pot

