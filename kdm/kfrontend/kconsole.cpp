    /*

    xconsole widget for KDM
    $Id$

    Copyright (C) 2002 Oswald Buddenhagen <ossi@kde.org>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include <config.h>

#ifdef __sgi
#define __svr4__
#endif

#if defined(HAVE_GRANTPT) && defined(HAVE_PTSNAME) && defined(HAVE_UNLOCKPT) && !defined(_XOPEN_SOURCE) && !defined(__svr4__)
#define _XOPEN_SOURCE // make stdlib.h offer the above fcts
#endif

/* for NSIG */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifdef __osf__
#define _OSF_SOURCE
//XXX #include <float.h>
#endif

#ifdef _AIX
#define _ALL_SOURCE
//XXX already in <config.h>  #include <sys/types.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#ifdef HAVE_TERMIOS_H
/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
#include <termios.h>
}
#endif
#if !defined(__osf__)
#ifdef HAVE_TERMIO_H
/* needed at least on AIX */
#include <termio.h>
#endif
#endif
#include <unistd.h>

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif

#if defined(HAVE_PTY_H)
#include <pty.h>
#endif

#include "kconsole.h"

#include "kdm_greet.h"

#include <klocale.h>

#include <qsocketnotifier.h>
#include <qregexp.h>

KConsole::KConsole( QWidget *_parent, const QString &src )
    : inherited( _parent )
    , source( src )
{
    setReadOnly( true );
    setWordWrap( NoWrap );
    setTextFormat( PlainText );

    if (!OpenConsole())
	append(i18n("Cannot open console\n"));
}

KConsole::~KConsole()
{
    CloseConsole();
}

int
KConsole::OpenConsole()
{
    if (!source.isEmpty()) {
	master_fd = open( source.latin1(), O_RDONLY | O_NONBLOCK );
	if (master_fd < 0)
	    LogError( "Cannot open log source %s, "
		      "falling back to /dev/console.\n", source.latin1());
	else {
	    slave_fd = -1;
	    goto gotcon;
	}
    }

#if defined(HAVE_OPENPTY)
    if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL)) {
	perror("openpty");
	return 0;
    }
#else
#if defined(__sgi__) || defined(__osf__) || defined(__svr4__)
//#if defined(HAVE_GRANTPT) && defined(HAVE_PTSNAME)
# ifdef _AIX
    master_fd = open("/dev/ptc", O_RDWR);
    if (master_fd < 0) {
	perror("opening /dev/ptc");
# else
    master_fd = open("/dev/ptmx", O_RDWR);
    if (master_fd < 0) {
	perror("opening /dev/ptmx");
# endif
	return 0;
    }
    char *ptsn;
    if (!(ptsn = ptsname(master_fd)) ||
	(slave_fd = open(ptsn, O_RDWR) < 0)) {
	perror("opening pseudo tty slave");
	::close(master_fd);
	return 0;
    }
    grantpt(master_fd);
#else
#if defined(_SCO_DS) || defined(__USLC__) // SCO OSr5 and UnixWare, might be obsolete
    for (int idx = 0; idx < 256; idx++) {
      char ptynam[32], ttynam[32];
      sprintf(ptynam, "/dev/ptyp%d", idx);
      sprintf(ttynam, "/dev/ttyp%d", idx);
      if ((master_fd = open (ptynam, O_RDWR)) >= 0) {
        if ((slave_fd = open (ttynam, O_RDWR)) >= 0)
    	  goto gotpty;
	else
	  ::close(master_fd);
      }
    }
    perror("opening pseudo tty");
    return 0;
  gotpty:
#else
    for (const char *s3 = "pqrstuvwxyzabcdefghijklmno"; *s3; s3++) {
      for (const char *s4 = "0123456789abcdefghijklmnopqrstuvwxyz"; *s4; s4++) {
        char ptynam[32], ttynam[32];
        sprintf(ptynam, "/dev/pty%c%c", *s3, *s4);
        sprintf(ttynam, "/dev/tty%c%c", *s3, *s4);
        if ((master_fd = open (ptynam, O_RDWR)) >= 0) {
          if ((slave_fd = open (ttynam, O_RDWR)) >= 0)
      	    goto gotpty;
	  else
	    ::close(master_fd);
        }
      }
    }
    perror("opening pseudo tty");
    return 0;
  gotpty:
#endif
#endif
#endif

#ifdef TIOCCONS
    char on;
    on = 1;
    if (ioctl(slave_fd, TIOCCONS, &on) < 0) {
	perror("opening pseudo tty");
	::close(slave_fd);
	::close(master_fd);
	return 0;
    }
#else
    int consfd;
    if ((consfd = open("/dev/console", O_RDONLY)) < 0) {
	perror("opening /dev/console");
	::close(slave_fd);
	::close(master_fd);
	return 0;
    }
    if (ioctl(consfd, SRIOCSREDIR, slave_fd) < 0) {
	perror("ioctl SRIOCSREDIR");
	::close(consfd);
	::close(slave_fd);
	::close(master_fd);
	return 0;
    }
    close(consfd);
#endif

  gotcon:
    notifier = new QSocketNotifier(master_fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), SLOT(slotData()));
    return 1;
}

void
KConsole::CloseConsole()
{
    delete notifier;
    ::close(slave_fd);
    ::close(master_fd);
}

void
KConsole::slotData()
{
    int n;
    char buffer[1024];

    if ((n = read(master_fd, buffer, sizeof(buffer))) <= 0) {
	CloseConsole();
	if (!n)
	    if (!OpenConsole())
		append(i18n("\n*** Console log broken ***\n"));
    } else {
	bool as = !verticalScrollBar()->isVisible() ||
		  (verticalScrollBar()->value() ==
		   verticalScrollBar()->maxValue());
	QString str( QString::fromLocal8Bit(buffer, n).replace(QRegExp("\r"), "") );
	int pos, opos;
	for (opos = 0; (pos = str.find('\n', opos)) >= 0; opos = pos + 1)
	    if (!leftover.isEmpty()) {
		append( leftover + str.mid( opos, pos - opos ) );
		leftover = QString::null;
	    } else
		append( str.mid( opos, pos - opos ) );
	leftover += str.mid( opos );
	if (as)
	    scrollToBottom();
    }
}

#include "kconsole.moc"
