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
    Revision 1.8  2000/04/13 01:55:36  charles
    Milestone 6.  All that's left now is being able to save.  That's also
    the most important part :)

    Question: if you open a full path in KConfig:
    	/opt/kde2/share/config/bleh   (I look for this file with KStdDir)

    How do I get this file's local version (~/.kde/share/config/bleh) that is,
    and safely.

    Revision 1.7  2000/04/10 04:56:07  charles
    Thought I'de commit before I break out for the day^H^H^Hnight.

    Revision 1.6  2000/04/09 22:33:56  charles
    Milestone 3 :)

    This is going a LOT faster than mozilla!

    Revision 1.5  2000/04/09 19:06:40  charles
    another milestone.
    rwilliam: it compiles here fine after a distclean, perhaps you have a stale
    Makefile?

    Revision 1.4  2000/04/09 05:57:05  charles
    Major milestone in progress of the new version of this applet.

    Aren't ya all proud of me? :D

    Revision 1.3  2000/04/09 02:21:38  charles
    Yeah Baby! this rewrite is going good... very slowly.  Now it's a matter of
    connecting the UI to it.  fun, fun!

    KConfig is still broken it seems.  I will patch it if nobody else does :)

    Revision 1.1  2000/04/08 22:50:45  charles
    Totally broken for a change in design.
    I'll start doing some "object oriented programming" now! Who would've
    thought? :)

    eventconfig.h will load up everything into memory, and then put it into
    the lists box, and even do the rest of the goop.  ohh yeah.

*/


#ifndef _EVENTCONFIG_H
#define _EVENTCONFIG_H

#include <kconfig.h>
#include <knotifyclient.h>
#include <qobject.h>
#include <qlist.h>
#include <qlistview.h>
#include <eventview.h>


class ProgramConfig;

/**
 * Contains a single event
 **/
class EventConfig : public QObject
{
Q_OBJECT
public:
	class EventListViewItem : public QListViewItem
	{
	public:
		EventListViewItem(EventConfig *ev);
		EventConfig *event;
	};

	EventConfig(const ProgramConfig *parent=0) {application=parent;}
	EventConfig(const EventConfig *old) {set(old);}
	
	void set(const EventConfig *old);

	void load(KConfig &conf);
	/**
	 * When I was selected to get shown with the EventView
	 **/
	void selected();
	/**
	 * reload the data from the configuration file
	 **/
	void reload();
	
	const ProgramConfig *application;
	int present;
	QString internalname;
	
	QString logfile;
	QString soundfile;
	
	QString description;
	QString friendly;
};

/**
 * Contains a single program
 **/
class ProgramConfig : public QObject
{
Q_OBJECT
public:
	class ProgramListViewItem : public QListViewItem
	{
	public:
		ProgramListViewItem(ProgramConfig *prog);
		ProgramConfig *program;
	};

	ProgramConfig();
	~ProgramConfig();
	/**
	 * Load the data for this class, and it's child Events
	 */
	void load(KConfig &conf);
	
	/**
	 * shows it to the GUI
	 **/
	void show();
public slots:
	/**
	 * Get myself shown
	 **/
	void selected();
	/**
	 * My chillens want to be shown :)
	 **/
	void selected(QListViewItem *_i);

public:
	QString configfile;
	QString appname;
	QString description;

	
	QList<EventConfig> eventlist;
};

/**
 * Contains all the programs
 **/
class Programs : public QObject
{
Q_OBJECT
public:
	Programs(EventView *_eventview=0, QListView *_programs=0,
	         QListView *_events=0);
	~Programs();
	
	/**
	 * To the GUI!!!!
	 **/
	void show();
	
	/**
	 * Turns $KDEDIR/share/apps/appname/bleh into
	 * $KDEHOME/share/apps/appname/bleh
	 */
	static localVersion(const QString &path);

public slots:
	/**
	 * One of the programs was selected
	 **/
	void selected(QListViewItem *_i);

public:
	static EventView *eventview;
	static QListView *programs;
	static QListView *events;
	
	QList<ProgramConfig> programlist;

};

#endif

