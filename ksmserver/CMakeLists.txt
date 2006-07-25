
project(ksmserver)
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kdmlib  )

########### next target ###############

set(ksmserver_KDEINIT_SRCS ${kdmlib_SRCS} 
   main.cpp 
   server.cpp 
   shutdowndlg.cpp 
   legacy.cpp 
   startup.cpp 
   shutdown.cpp 
   client.cpp)

qt4_add_dbus_adaptor( ksmserver_KDEINIT_SRCS org.kde.KSMServerInterface.xml server.h KSMServer )

kde4_automoc(${ksmserver_KDEINIT_SRCS})


kde4_add_kdeinit_executable( ksmserver ${ksmserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksmserver ${KDE4_KDEUI_LIBS} ${KDE4_KDEFX_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${X11_LIBRARIES})

install(TARGETS kdeinit_ksmserver  DESTINATION ${LIB_INSTALL_DIR})

target_link_libraries( ksmserver kdeinit_ksmserver )
install(TARGETS ksmserver DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

if(KDE4_BUILD_TESTS)
set(testsh_SRCS ${kdmlib_SRCS} test.cpp shutdowndlg.cpp )

kde4_automoc(${testsh_SRCS})


kde4_add_executable(testsh ${testsh_SRCS})

target_link_libraries(testsh ${KDE4_KDEUI_LIBS})

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES shutdownkonq.png  DESTINATION  ${DATA_INSTALL_DIR}/ksmserver/pics )
install( FILES ksmserver.upd  move_session_config.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )




#original Makefile.am contents follow:

## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
## AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
## CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#SUBDIRS = .
#
#INCLUDES= -I$(top_srcdir)/kdmlib -I$(srcdir)/../lib $(all_includes)
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = ksmserver.la
#noinst_HEADERS = global.h server.h
#
#ksmserver_la_METASOURCES = AUTO
## Order is important for --enable-final!
#ksmserver_la_SOURCES = main.cpp server.cpp shutdowndlg.cpp \
#	legacy.cpp startup.cpp shutdown.cpp client.cpp \
#	KSMServerInterface.skel server.skel
#
#ksmserver_la_LDFLAGS = $(all_libraries) -avoid-version -module
#ksmserver_la_LIBADD = ../../kdmlib/libdmctl.la $(LIB_KDEUI)
#
#picsdir = $(kde_datadir)/ksmserver/pics
#pics_DATA = shutdownkonq.png
#
#update_DATA = ksmserver.upd
#update_SCRIPTS = move_session_config.sh
#updatedir = $(kde_datadir)/kconf_update
#
#
#EXTRA_PROGRAMS = testsh
#testsh_SOURCES = test.cpp
#testsh_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testsh_LDADD = $(LIB_KDEUI) shutdowndlg.lo ../../kdmlib/libdmctl.la
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/ksmserver.pot
