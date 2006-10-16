/***************************************************************************
 *   Copyright (C) 2005-2006 by Stephen Leaf <smileaf@smileaf.org>         *
 *   Copyright (C) 2006 by Oswald Buddenhagen <ossi@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef _KDMTHEME_H_
#define _KDMTHEME_H_

#include <QWidget>

class ThemeData;

class QCheckBox;
class QLabel;
class QPushButton;
class QTreeWidget;

class KDMThemeWidget : public QWidget {
	Q_OBJECT

  public:
	KDMThemeWidget( QWidget *parent = 0 );

	void load();
	void save();
	void defaults();
	void makeReadOnly();

  private:
	void selectTheme( const QString & );
	void insertTheme( const QString & );
	void updateInfoView( ThemeData * );

	QTreeWidget *themeWidget;
	QLabel *preview;
	QLabel *info;
	QPushButton *bInstallTheme;
	QPushButton *bRemoveTheme;

	ThemeData *defaultTheme;
	QString themeDir;

  Q_SIGNALS:
	void changed();

  protected Q_SLOTS:
	void themeSelected();
	void removeSelectedThemes();
	void installNewTheme();

};

#endif
