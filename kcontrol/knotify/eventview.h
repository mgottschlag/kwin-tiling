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
    Revision 1.15  2000/05/25 21:13:36  charles
    *** empty log message ***

    Revision 1.14  2000/05/21 08:23:22  charles
    *** empty log message ***

    Revision 1.13  2000/04/13 01:55:36  charles
    Milestone 6.  All that's left now is being able to save.  That's also
    the most important part :)

    Question: if you open a full path in KConfig:
    	/opt/kde2/share/config/bleh   (I look for this file with KStdDir)

    How do I get this file's local version (~/.kde/share/config/bleh) that is,
    and safely.

    Revision 1.12  2000/04/11 05:33:12  charles
    Milestone 5.  Can't even remember how it's better, but it is (trust me).

    Revision 1.11  2000/04/10 23:34:06  charles
    More stable, supports browsing better.
    Still doesn't save the configuration though :(
    I'm not done with this one yet!

    Revision 1.10  2000/04/10 04:56:07  charles
    Thought I'de commit before I break out for the day^H^H^Hnight.

    Revision 1.9  2000/04/09 22:33:57  charles
    Milestone 3 :)

    This is going a LOT faster than mozilla!

    Revision 1.8  2000/04/08 22:50:45  charles
    Totally broken for a change in design.
    I'll start doing some "object oriented programming" now! Who would've
    thought? :)

    eventconfig.h will load up everything into memory, and then put it into
    the lists box, and even do the rest of the goop.  ohh yeah.

    Revision 1.7  2000/04/01 00:08:03  faure
    first half of the fix

    Revision 1.6  2000/03/25 17:27:14  charles
    It's going.  It's also very messy :|
    At least it works...

    Revision 1.5  2000/03/25 06:37:24  charles
    Slight rework so this isn't a pain in the butt to code :)

    Revision 1.4  2000/03/25 00:01:08  charles
    Now shows whether the events are toggled. and klined.h->klineedit.h

    I need an artist to draw me a "enable"/"disable" icon.  Right now I'm using
    either a flag or nothing.

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

#include <knotifyclient.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <qpushbutton.h>

class EventConfig;

class EventView : public QWidget
{
Q_OBJECT

public:
	EventView(QWidget *parent, const char *name=0);
	virtual ~EventView();

	int enumNum(int listNum);
	
public slots:
	void defaults();
	void load(EventConfig *_event, bool save=true);
	void unload(bool save=true);
	void itemSelected(int item);
	
protected slots:
	void setPixmaps();
	void itemToggled(bool on);

	void textChanged(const QString &str);
	void playSound();

signals:
	void changed();

private:
	QPushButton *play;
	QListBox *eventslist;
	QCheckBox *enabled;
	KURLRequester *file;
	QPushButton *todefault;
	
	EventConfig *event;
	int oldListItem;
};

#endif
