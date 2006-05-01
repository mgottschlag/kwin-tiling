INCLUDES = -I$(srcdir)/../libkicker -I../libkicker $(all_includes)

lib_LTLIBRARIES = libtaskmanager.la
libtaskmanager_la_SOURCES = tasklmbmenu.cpp taskrmbmenu.cpp taskmanager.cpp
libtaskmanager_la_METASOURCES = AUTO

libtaskmanager_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libtaskmanager_la_LIBADD = $(LIB_KDECORE) $(LIB_XFIXES) $(LIB_XRENDER) $(LIB_XCOMPOSITE) ../libkicker/libkickermain.la

