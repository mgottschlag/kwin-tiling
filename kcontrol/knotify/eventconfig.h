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
    Revision 1.1  2000/04/08 22:50:45  charles
    Totally broken for a change in design.
    I'll start doing some "object oriented programming" now! Who would've
    thought? :)

    eventconfig.h will load up everything into memory, and then put it into
    the lists box, and even do the rest of the goop.  ohh yeah.

*/


#ifndef _EVENTCONFIG_H
#define _EVENTCONFIG_H

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
	EventConfig(const ProgramConfig *parent=0) {app=parent;}
	
	const ProgramConfig *app;
	int present;
	QString logfile;
	QString soundfile;
	
	QString description;
	QString friendly;
};

class ProgramConfig
{
public:
	ProgramConfig() {}
	~ProgramConfig();
	
	QString configfile;
	QString appname;
	QString description;
	
	QList<EventConfig> events;
};

class Programs
{
	Programs() {};
	
	static EventView *eventview;
	static QListView *programs;
	static QListView *events;

};

#endif

