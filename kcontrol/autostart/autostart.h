/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@smileaf.org                                                   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef _AUTOSTART_H_
#define _AUTOSTART_H_

#include <KCModule>
#include <KAboutData>
#include <KGlobalSettings>
#include <QComboBox>
#include <QPushButton>
#include <QTreeWidget>

#include "ui_autostartconfig.h"

class desktop;

class Autostart: public KCModule
{
    Q_OBJECT

public:
    Autostart( QWidget* parent, const QStringList&  );
    ~Autostart();

    void load();
    void save();
    void defaults();
    QStringList paths;
    QStringList pathName;

public slots:
	void addCMD();
	void removeCMD();
	void editCMD(QTreeWidgetItem*);
	bool editCMD(KFileItem);
	void editCMD();
	void setStartOn(int);
	void selectionChanged();

private:
	Ui_AutostartConfig *widget;
	KAboutData *myAboutData;
	KGlobalSettings *kgs;
};

#endif
