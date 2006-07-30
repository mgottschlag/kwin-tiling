kconf_PROGRAMS = khotkeys_update
kconfdir = $(libdir)/kconf_update_bin

khotkeys_update_SOURCES = update.cpp

khotkeys_update_LDADD   = ../shared/libkhotkeys_shared.la $(LIB_KDECORE) $(KDE_RPATH)
khotkeys_update_LDFLAGS = $(all_libraries)

INCLUDES = -I$(srcdir)/../shared $(all_includes)

METASOURCES = AUTO
