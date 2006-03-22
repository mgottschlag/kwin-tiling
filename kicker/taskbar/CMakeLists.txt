kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker ${CMAKE_SOURCE_DIR}/workspace/kicker/taskmanager )


########### next target ###############

set(taskbar_LIB_SRCS taskcontainer.cpp taskbar.cpp )

kde4_automoc(${taskbar_LIB_SRCS})

kde4_add_dcop_skels(taskbar_LIB_SRCS taskbar.h )

kde4_add_kcfg_files(taskbar_LIB_SRCS taskbarsettings.kcfgc )

kde4_add_library(taskbar SHARED ${taskbar_LIB_SRCS})

target_link_libraries(taskbar  ${KDE4_KDEUI_LIBS} kdefx kutils kickermain taskmanager )

set_target_properties(taskbar PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} taskbar )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES taskbar.kcfg )

kde4_footer()



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
