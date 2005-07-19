/*
 *  tzone.h
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef tzone_included
#define tzone_included

#include <ktimezones.h>
#include <ktimezonewidget.h>
#include <qvgroupbox.h>

class QComboBox;
class QLabel;

class Tzone : public QVGroupBox
{
  Q_OBJECT

public:
  Tzone( QWidget *parent=0, const char* name=0 );

  void	save();
  void  load();

signals:
	void zoneChanged(bool);

protected slots:
  void handleZoneChange() {emit zoneChanged( TRUE );}

private:
  void currentZone();
  KTimezones m_zoneDb;
  QLabel *m_local;
  KTimezoneWidget *tzonelist;
};

#endif // tzone_included
