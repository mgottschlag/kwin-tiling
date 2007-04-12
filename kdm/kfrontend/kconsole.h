/*

xconsole widget for KDM

Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KCONSOLE_H
#define KCONSOLE_H

#include <Qt3Support/Q3TextEdit>

class KPty;
class QSocketNotifier;

class KConsole : public Q3TextEdit {
	Q_OBJECT
	typedef Q3TextEdit inherited;

  public:
	KConsole( QWidget *_parent = 0 );
	~KConsole();

  private Q_SLOTS:
	void slotData();

  private:
	int OpenConsole();
	void CloseConsole();

	KPty *pty;
	QSocketNotifier *notifier;
	QString leftover;
	int fd;
};

#endif // KCONSOLE_H
