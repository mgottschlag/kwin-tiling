/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/  

#ifndef __KDMSESS_H__
#define __KDMSESS_H__


#include <qstring.h>

class QComboBox;
class QCheckBox;
class KURLRequester;
class KBackedComboBox;

class KDMSessionsWidget : public QWidget
{
	Q_OBJECT

public:
	KDMSessionsWidget(QWidget *parent=0, const char *name=0);

	void load();
	void save();
	void defaults();
	void makeReadOnly();

	enum SdModes { SdAll, SdRoot, SdNone };

Q_SIGNALS:
	void changed( bool state );
	
protected Q_SLOTS:
	void changed();

private:
	void readSD (QComboBox *, QString);
	void writeSD (QComboBox *);

	QComboBox	*sdlcombo, *sdrcombo;
	QLabel		*sdllabel, *sdrlabel;
	KURLRequester	*restart_lined, *shutdown_lined;
	KBackedComboBox	*bm_combo;
};


#endif


