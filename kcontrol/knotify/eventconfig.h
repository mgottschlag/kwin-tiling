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
	
	void load(KConfig &conf);
	
	
	const ProgramConfig *application;
	int present;
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

	ProgramConfig() {}
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
	void selected();

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

public slots:
	void selected(QListViewItem *_i);

public:
	static EventView *eventview;
	static QListView *programs;
	static QListView *events;
	
	QList<ProgramConfig> programlist;

};

#endif

