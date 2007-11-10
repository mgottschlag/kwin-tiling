/*

Copyright (C) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2002,2004 Oswald Buddenhagen <ossi@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#ifndef __KDM_KROOTIMAGE_H__
#define __KDM_KROOTIMAGE_H__

#include <bgrender.h>

#include <QApplication>
#include <QTimer>

class MyApplication : public QApplication
{
	Q_OBJECT

  public:
	MyApplication( const char *conf, int argc, char **argv );

  private Q_SLOTS:
	void renderDone();
	void slotTimeout();

  private:
	KVirtualBGRenderer renderer;
	QTimer timer;
};

#endif // __KDM_KROOTIMAGE_H__
