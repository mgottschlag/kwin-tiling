kconf_PROGRAMS = khotkeys_update
kconfdir = $(kde_bindir)/kconf_update_bin

khotkeys_update_SOURCES = update.cpp

khotkeys_update_LDADD   = ../shared/libshared.la $(LIB_KDECORE)
khotkeys_update_LDFLAGS = $(all_libraries)

noinst_HEADERS =

INCLUDES = -I$(srcdir)/../shared $(all_includes)

METASOURCES = AUTO
