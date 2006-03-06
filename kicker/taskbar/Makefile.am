INCLUDES = -I$(srcdir)/../taskmanager -I$(srcdir)/../libkicker -I../libkicker $(all_includes)

# KDE 4: rename to libkickertaskbar.la!

lib_LTLIBRARIES = libtaskbar.la
libtaskbar_la_SOURCES = taskcontainer.cpp taskbar.cpp \
                        taskbar.skel \
                        taskbarsettings.kcfgc

libtaskbar_la_METASOURCES = AUTO

libtaskbar_la_LDFLAGS = -version-info 3:0:2 $(all_libraries) -no-undefined
libtaskbar_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS) ../libkicker/libkickermain.la ../taskmanager/libtaskmanager.la

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libtaskbar.pot

taskbar.lo: ../libkicker/kickerSettings.h
taskbarcontainer.lo: ../libkicker/kickerSettings.h

kde_kcfg_DATA = taskbar.kcfg
