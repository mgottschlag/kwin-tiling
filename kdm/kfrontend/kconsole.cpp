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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif


#include "kconsole.h"
#include "kdm_greet.h"

#include <klocale.h>
#include <kpty.h>

#include <qsocketnotifier.h>

KConsole::KConsole( QWidget *_parent, const QString &src )
    : inherited( _parent )
    , pty( 0 )
    , notifier( 0 )
    , source( src )
    , fd( -1 )
{
    setReadOnly( true );
    setWordWrap( NoWrap );
    setTextFormat( PlainText );

    if (!OpenConsole())
	append(i18n("Cannot open console"));
}

KConsole::~KConsole()
{
    CloseConsole();
}

int
KConsole::OpenConsole()
{
    if (!source.isEmpty()) {
	if ((fd = open( source.latin1(), O_RDONLY | O_NONBLOCK )) >= 0)
	    goto gotcon;
	LogError( "Cannot open log source %s, "
		  "falling back to /dev/console.\n", source.latin1() );
    }

    pty = new KPty;
    if (!pty->open()) {
	delete pty;
	pty = 0;
	return 0;
    }

#ifdef TIOCCONS
    static const char on = 1;
    if (ioctl(pty->slaveFd(), TIOCCONS, &on) < 0) {
	perror("ioctl TIOCCONS");
	delete pty;
	pty = 0;
	return 0;
    }
#else
    int consfd;
    if ((consfd = open("/dev/console", O_RDONLY)) < 0) {
	perror("opening /dev/console");
	delete pty;
	pty = 0;
	return 0;
    }
    if (ioctl(consfd, SRIOCSREDIR, slave_fd) < 0) {
	perror("ioctl SRIOCSREDIR");
	::close(consfd);
	delete pty;
	pty = 0;
	return 0;
    }
    ::close(consfd);
#endif
    fd = pty->masterFd();

  gotcon:
    notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), SLOT(slotData()));
    return 1;
}

void
KConsole::CloseConsole()
{
    delete notifier;
    notifier = 0;
    if (pty) {
	delete pty;
	pty = 0;
    } else
	::close (fd);
    fd = -1;
}

void
KConsole::slotData()
{
    int n;
    char buffer[1024];

    if ((n = read(fd, buffer, sizeof(buffer))) <= 0) {
	CloseConsole();
	if (!n)
	    if (!OpenConsole())
		append(i18n("\n*** Cannot open console log source ***"));
    } else {
	bool as = !verticalScrollBar()->isVisible() ||
		  (verticalScrollBar()->value() ==
		   verticalScrollBar()->maxValue());
	QString str( QString::fromLocal8Bit(buffer, n).remove('\r') );
	int pos, opos;
	for (opos = 0; (pos = str.find('\n', opos)) >= 0; opos = pos + 1) {
	    if (paragraphs() == 100)
		removeParagraph( 0 );
	    if (!leftover.isEmpty()) {
		append( leftover + str.mid( opos, pos - opos ) );
		leftover = QString::null;
	    } else
		append( str.mid( opos, pos - opos ) );
	}
	leftover += str.mid( opos );
	if (as)
	    scrollToBottom();
    }
}

#include "kconsole.moc"
