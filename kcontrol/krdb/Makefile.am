AM_CPPFLAGS = $(all_includes)

SUBDIRS = . themes kcs ad pics

noinst_LTLIBRARIES = libkrdb.la
libkrdb_la_SOURCES = krdb.cpp
libkrdb_la_LIBADD = $(LIB_KDECORE)
libkrdb_la_LDFLAGS = -no-undefined $(all_libraries) 


update_DATA = kcmdisplayrc.upd
updatedir = $(kde_datadir)/kconf_update
