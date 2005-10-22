# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SUBDIRS = .

INCLUDES= -I$(top_srcdir)/kdmlib -I$(srcdir)/../lib $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = ksmserver.la
noinst_HEADERS = global.h server.h shutdown.h

ksmserver_la_METASOURCES = AUTO
# Order is important for --enable-final!
ksmserver_la_SOURCES = main.cpp server.cpp shutdown.cpp KSMServerInterface.skel server.skel

ksmserver_la_LDFLAGS = $(all_libraries) -avoid-version -module
ksmserver_la_LIBADD = ../../kdmlib/libdmctl.la $(LIB_KDEUI)

picsdir = $(kde_datadir)/ksmserver/pics
pics_DATA = shutdownkonq.png

update_DATA = ksmserver.upd
update_SCRIPTS = move_session_config.sh
updatedir = $(kde_datadir)/kconf_update


EXTRA_PROGRAMS = testsh
testsh_SOURCES = test.cpp
testsh_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testsh_LDADD = $(LIB_KDEUI) shutdown.lo ../../kdmlib/libdmctl.la

messages:
	$(XGETTEXT) *.cpp -o $(podir)/ksmserver.pot
