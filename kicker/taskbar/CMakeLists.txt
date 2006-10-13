
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/taskmanager  )


########### next target ###############

set(taskbar_LIB_SRCS taskcontainer.cpp taskbar.cpp )

kde4_automoc(taskbar ${taskbar_LIB_SRCS})


kde4_add_kcfg_files(taskbar_LIB_SRCS taskbarsettings.kcfgc )

kde4_add_library(taskbar SHARED ${taskbar_LIB_SRCS})

target_link_libraries(taskbar ${KDE4_KDEUI_LIBS} ${KDE4_KDEFX_LIBS} ${KDE4_KUTILS_LIBS} kickermain taskmanager )

set_target_properties(taskbar PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install(TARGETS taskbar  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES taskbar.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )




#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../taskmanager -I$(srcdir)/../libkicker -I$(top_builddir)/kicker/libkicker $(all_includes)
#
## KDE 4: rename to libkickertaskbar.la!
#
#lib_LTLIBRARIES = libtaskbar.la
#libtaskbar_la_SOURCES = taskcontainer.cpp taskbar.cpp \
#                        taskbar.skel \
#                        taskbarsettings.kcfgc
#
#libtaskbar_la_METASOURCES = AUTO
#
#libtaskbar_la_LDFLAGS = -version-info 3:0:2 $(all_libraries) -no-undefined
#libtaskbar_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS) ../libkicker/libkickermain.la ../taskmanager/libtaskmanager.la
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libtaskbar.pot
#
#taskbar.lo: ../libkicker/kickerSettings.h
#taskbarcontainer.lo: ../libkicker/kickerSettings.h
#
#kde_kcfg_DATA = taskbar.kcfg
