    /*

    Front-end to Lilo's boot options
    $Id$

    Copyright (C) 1999 Stefan van den Oord <oord@cs.utwente.nl>
    Copyright (C) 2001,2003 Oswald Buddenhagen <ossi@kde.org>

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

#if defined(__linux__) && defined(__i386__)

#include "liloinfo.h"

#include <qfileinfo.h>
#include <qcstring.h>

LiloInfo::LiloInfo ( const QString &lilolocation,
		     const QString &bootmaplocation ) :
	liloloc( lilolocation ),
	bootmaploc( bootmaplocation ),
	indexDefault( -1 )
{
	if ( !QFileInfo ( liloloc ).exists() )
		error = -1;
	else if ( !QFileInfo ( bootmaploc ).exists() )
		error = -2;
	else
	{
		LiloProcess liloproc( liloloc, bootmaploc );
		liloproc << "-q" << "-v";

		if ( !liloproc.exec() )
		{
			error = -3;
			return;
		}

		liloErrorString = liloproc.error();
		if (!liloErrorString.isEmpty())
		{
			error = -4;
			return;
		}

		QStringList lst = liloproc.stdout();
		QStringList::Iterator it = lst.begin();

		for (;; ++it)
		{
			if (it == lst.end() )
			{
				error = -5;
				return;
			}
			if (*it == "Images:")
				break;
			if ((*it).startsWith( "  Default boot command line:" ))
				nextOption = (*it).mid( 30, (*it).length() - 31 );
		}

		while (++it != lst.end())
		{
			if ((*it)[0] == ' ' && (*it)[1] == ' ' && (*it)[2] != ' ')
				options.append( *it );
		}

		options.sort();
		int i = 0;
		for ( it = options.begin(); it != options.end(); ++it, ++i )
		{
			if ( (*it)[ (*it).length() - 1 ] == '*' )
			{
				indexDefault = i;
				(*it).truncate( (*it).length() - 1 );
			}
			*it = (*it).stripWhiteSpace();
		}
		error = 0;
	}
}

int LiloInfo::getBootOptions( QStringList &bootOptions, int &defaultOptionIndex )
{
	bootOptions = options;
	defaultOptionIndex = indexDefault;
	return error;
}

int LiloInfo::getNextBootOption ( QString &nextBootOption )
{
	nextBootOption = nextOption;
	return error;
}

int LiloInfo::setNextBootOption ( const QString &nextBootOption )
{
	if ( error )
		return error;

	LiloProcess liloproc( liloloc, bootmaploc );
	liloproc << "-R" << nextBootOption;

	if ( !liloproc.exec() )
		return -3;

	liloErrorString = liloproc.error();
	if (!liloErrorString.isEmpty())
		return -4;

	return 0;
}


LiloProcess::LiloProcess ( const QString &lilolocation,
			   const QString &bootmaplocation ) :
    KProcess()
{
    *this << lilolocation << "-m" << bootmaplocation << "-w";

    connect ( this, SIGNAL(receivedStdout(KProcess *, char *, int)),
	      SLOT(processStdout(KProcess *, char *, int)) );
    connect ( this, SIGNAL(receivedStderr(KProcess *, char *, int)),
	      SLOT(processStderr(KProcess *, char *, int)) );
}

bool LiloProcess::exec()
{
    return start ( Block, (Communication) ( Stdout | Stderr ) );
}

QStringList LiloProcess::stdout()
{
    return QStringList::split( '\n', QString::fromLocal8Bit( _stdout.data(), _stdout.size() ) );
}

QString LiloProcess::error()
{
    QStringList errs( QStringList::split( '\n', QString::fromLocal8Bit( _stderr.data(), _stderr.size() ) ) );
    for (QStringList::Iterator it = errs.begin(); it != errs.end(); ) {
	if ( (*it).startsWith( "Ignoring " ) ) {
	    it = errs.remove( it );
	    continue;
	}
	++it;
    }
    return errs.join( "\n" );
}

static void append( QByteArray &arr, const char *buf, int len )
{
    int l1 = arr.size();
    if (arr.resize( l1 + len ))
	memcpy( arr.data() + l1, buf, len );
}

void LiloProcess::processStdout ( KProcess *, char *buffer, int len )
{
    append( _stdout, buffer, len );
}

void LiloProcess::processStderr ( KProcess *, char *buffer, int len )
{
    append( _stderr, buffer, len );
}

#include "liloinfo.moc"

#endif
