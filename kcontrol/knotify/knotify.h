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
    Revision 1.8  2000/04/15 19:46:06  charles
    Here's a commit. Happy David? :)

    Oh. yeah. it does something.  It's done.  Well, unless you don't speak english.
    I'm working on that now :)

    Revision 1.7  2000/04/11 05:33:13  charles
    Milestone 5.  Can't even remember how it's better, but it is (trust me).

    Revision 1.6  2000/04/09 05:57:05  charles
    Major milestone in progress of the new version of this applet.

    Aren't ya all proud of me? :D

    Revision 1.5  2000/04/08 22:50:45  charles
    Totally broken for a change in design.
    I'll start doing some "object oriented programming" now! Who would've
    thought? :)

    eventconfig.h will load up everything into memory, and then put it into
    the lists box, and even do the rest of the goop.  ohh yeah.

    Revision 1.4  2000/03/23 02:51:51  charles
    Progressivly getting to the level of "usable" :)

    Revision 1.3  2000/03/21 23:42:51  charles
    Can anyone try to get the Layout to work properly? Is this a QT bug?
    Is this my own fault?

    It's horribly huge.

    Oh, and it lists the programs properly (thanks coolo!).
    That means that every program can/should now officially create
    $KDEDIR/share/apps/appname/eventsrc

    Or I shall stabilize the API first :D

    Revision 1.2  2000/03/19 07:23:28  charles
    the module actually "exists" now :D
    Just wait until I start to DO something with it!

    And how do you debug these darned things?
    cd knotify
    make --dammit it_work

    Revision 1.1  2000/03/19 01:32:22  charles
    A rediculously early commit so that I can rm -rf all I want :)
    and, btw, applnk/Settings/System/Makefile.am is unchanged :)

    This is all for the sake of KNotify.

    I'm gonna finish this a lot sooner than I thought I would!

*/  


#ifndef _KNOTIFY_H
#define _KNOTIFY_H

#include "kcmodule.h"

#include <qstringlist.h>
#include <qstring.h>
#include <qlistview.h>
#include <qcheckbox.h>

#include <kaboutdata.h>

#include "eventview.h"

class Programs;



class KNotifyWidget : public KCModule
{
Q_OBJECT

public:
	KNotifyWidget(QWidget *parent, const char *name);
	virtual ~KNotifyWidget();

	void defaults();
	virtual void save();
	virtual QString quickHelp();
	virtual const KAboutData *aboutData() const;
		
private slots:
	void changed();
	/**
	 * Load all the apps
	 */
	void loadAll();
	
protected:
	QListView *apps;
	QListView *events;
	EventView *eventview;

	Programs *applications;
	
};

#endif
