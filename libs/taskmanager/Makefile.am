INCLUDES = $(all_includes) -I$(srcdir)/../share

lib_LTLIBRARIES = libtaskmanager.la
libtaskmanager_la_SOURCES = tasklmbmenu.cpp taskrmbmenu.cpp taskmanager.cpp
libtaskmanager_la_METASOURCES = AUTO

libtaskmanager_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libtaskmanager_la_LIBADD = $(LIB_KDECORE) $(LIB_XFIXES) $(LIB_XRENDER) $(LIB_XCOMPOSITE) ../share/libkickermain.la

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/libtaskmanager.pot
