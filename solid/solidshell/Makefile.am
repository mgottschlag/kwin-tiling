INCLUDES = -I$(srcdir)/../../kdehw $(all_includes)

bin_PROGRAMS = solidshell

solidshell_SOURCES = main.cpp
solidshell_LDFLAGS = $(all_libraries) $(KDE_RPATH)
solidshell_LDADD = ../../kdehw/libkdehw.la

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/solidshell.pot

METASOURCES = AUTO

