SUBDIRS = config pics

INCLUDES = -I$(srcdir)/../backend $(all_includes)

bin_PROGRAMS = kdm_config kdm_greet

kdm_config_SOURCES = kdm_config.c

kdm_greet_SOURCES = \
    kdm_greet.c \
    kdmconfig.cpp \
    kdmshutdown.cpp \
    kfdialog.cpp \
    kdmclock.cpp \
    kgreeter.cpp \
    liloinfo.cpp
kdm_greet_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdm_greet_LDADD = $(LIB_QT) $(LIB_KDECORE) $(LIBXINERAMA) $(LIB_KDEUI)

METASOURCES = kdmshutdown.moc kdmclock.moc kgreeter.moc liloinfo.moc kfdialog.moc

messages:
	$(XGETTEXT) $(kdm_greet_SOURCES) -o $(podir)/kdmgreet.pot

noinst_HEADERS = kdm_greet.h kdm_config.h kdmshutdown.h kdmclock.h kgreeter.h liloinfo.h kfdialog.h kdmconfig.h

