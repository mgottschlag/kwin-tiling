/* 

    $Id$

    Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Log$
    Revision 1.3  2000/03/24 06:01:25  charles
    More mindless boring stuff.

    Revision 1.2  2000/03/23 02:51:51  charles
    Progressivly getting to the level of "usable" :)

    Revision 1.1  2000/03/19 07:23:28  charles
    the module actually "exists" now :D
    Just wait until I start to DO something with it!

    And how do you debug these darned things?
    cd knotify
    make --dammit it_work

*/  


#ifndef _EVENTVIEW_H
#define _EVENTVIEW_H

#include <qlistbox.h>
#include <qcheckbox.h>
#include <klineedit.h>
#include <kconfig.h>
#include <qpushbutton.h>

class EventView : public QWidget
{
Q_OBJECT

public:
	EventView(QWidget *parent, const char *name=0);
	virtual ~EventView();

public slots:
	void defaults();
	void load(KConfig *config, const QString &section);
	void save();
	void unload();

protected slots:
	void setPixmap(int item, bool on);

signals:
	void changed();

protected:
	KConfig *conf;
	QString section;
	QListBox *eventslist;
	QCheckBox *enabled;
	KLineEdit *file;
	QPushButton *todefault;
};

#endif
