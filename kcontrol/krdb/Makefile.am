# $Id$

INCLUDES = $(all_includes)

SUBDIRS = . kcs ad pics


noinst_LTLIBRARIES = libkrdb.la
libkrdb_la_SOURCES = krdb.cpp
libkrdb_la_LIBADD = $(LIB_KDECORE)
libkrdb_la_LDFLAGS = -no-undefined $(all_libraries) 

messages:

update_DATA = kcmdisplayrc.upd
updatedir = $(kde_datadir)/kconf_update
