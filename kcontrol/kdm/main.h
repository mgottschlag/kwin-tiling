/*
 * main.h
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __kdm_main_h
#define __kdm_main_h

#include <kcmodule.h>

#include <QMap>

class KDMGeneralWidget;
class KDMDialogWidget;
class KDMThemeWidget;
class KDMSessionsWidget;
class KDMUsersWidget;
class KDMConvenienceWidget;
class KBackground;

class QStackedWidget;
class QTabWidget;

class KDModule : public KCModule {
	Q_OBJECT

  public:
	KDModule( QWidget *parent, const QStringList & );
	~KDModule();

	void load();
	void save();
	void defaults();

  public Q_SLOTS:
	void slotMinMaxUID( int min, int max );
	void slotUseThemeChanged( bool use );

  Q_SIGNALS:
	void clearUsers();
	void addUsers( const QMap<QString,int> & );
	void delUsers( const QMap<QString,int> & );

  private:
	QTabWidget *tab;

	KDMGeneralWidget *general;
	KDMDialogWidget *dialog;
	KBackground *background;
	KDMThemeWidget *theme;
	KDMSessionsWidget *sessions;
	KDMUsersWidget *users;
	KDMConvenienceWidget *convenience;
	QStackedWidget *dialog_stack;
	QStackedWidget *background_stack;
	QStackedWidget *theme_stack;

	QMap<QString, QPair<int,QStringList> > usermap;
	QMap<QString,int> groupmap;
	int minshowuid, maxshowuid;
	bool updateOK;

	void propagateUsers();

};

#endif

