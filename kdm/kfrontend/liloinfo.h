    /*

    Front-end to Lilo's boot options

    Copyright (C) 1999 Stefan van den Oord <oord@cs.utwente.nl>
    Copyright (C) 2001 Oswald Buddenhagen <ossi@kde.org>

    NOTE: this is a heavily cut-down version of the liloinfo class.
    Check out kde 2.1 for the full version.

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

#ifndef LILOINFO_H
#define LILOINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qstring.h>
#include <qstringlist.h>

#include <kprocess.h>

/*
 * Errors:
 *      0 = No error
 *     -1 = Lilo location is a non-existing file
 *     -2 = Bootmap location is a non-existing file
 *     -3 = Failed to run Lilo
 *     -4 = Lilo Error (description in getErrorDescription)
 *     -5 = Error parsing Lilo output
 */

class LiloInfo
{
	public:
		LiloInfo ( const QString &lilolocation,
			   const QString &bootmaplocation );

		int getBootOptions ( QStringList &bootOptions, int &defaultOptionIndex );
		int getNextBootOption ( QString &nextBootOption );
		int setNextBootOption ( const QString &nextBootOption );
//		QString getErrorDescription() { return liloErrorString; }

	private:
		QString liloloc, bootmaploc;
		QStringList options;
		int indexDefault;
		QString nextOption;
		int error;
		QString liloErrorString;

};

class LiloProcess : public KProcess
{
	Q_OBJECT

	public:
		LiloProcess ( const QString &lilolocation,
			      const QString &bootmaplocation );
		bool exec();
		QStringList getStdout();
		QString error();

	private:
		QByteArray _stdout, _stderr;

	private slots:
		void processStdout ( KProcess *, char *buffer, int len );
		void processStderr ( KProcess *, char *buffer, int len );
};

#endif // LILOINFO_H
