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

*/
#include "eventview.h"
#include <qlistview.h>
#include <eventconfig.h>
#include <kstddirs.h>
#include "eventconfig.moc"

EventView *Programs::eventview=0;
QListView *Programs::programs=0;
QListView *Programs::events=0;

void EventConfig::load(KConfig &conf)
{
	internalname=conf.group();
	friendly=conf.readEntry("Name", "Unknown Name");
	description=conf.readEntry("Comment", "No Description");
	
	{ // Load the presentation
		present=conf.readNumEntry("presentation", -1);
		if (present==-1)
			present=conf.readNumEntry("default_presentation", 0);
	}

	{ // Load the files
		soundfile=conf.readEntry("soundfile");
		if (soundfile.isNull())
			soundfile=conf.readEntry("default_soundfile");
	}
	
	{ // Load the files
		logfile=conf.readEntry("logfile");
		if (logfile.isNull())
			logfile=conf.readEntry("default_logfile");
	}
}

void EventConfig::set(const EventConfig *old)
{
	internalname=old->internalname;
	application=old->application;
	present=old->present;
	logfile=old->logfile;
	soundfile=old->soundfile;
	description=old->description;
	friendly=old->friendly;
}

void EventConfig::reload()
{
	KConfig conf(application->configfile);
	conf.setGroup(internalname);
	load(conf);

}

ProgramConfig::~ProgramConfig()
{
	eventlist.setAutoDelete(true);
}

void ProgramConfig::load(KConfig &conf)
{
	// Load the Names
	appname=conf.readEntry("Name", "Unknown Title");
	description=conf.readEntry("Comment", "No Description");

	// Load all the events	
	QStringList conflist=conf.groupList();
	conflist.remove(QString("!Global!"));
	conflist.remove(QString("<default>"));
	
	for (QStringList::Iterator it=conflist.begin(); it!=conflist.end(); ++it)
	{
		conf.setGroup(*it);
		EventConfig *e=new EventConfig(this);
		e->load(conf);
		eventlist.append(e);
		kapp->processEvents();
	}
}

Programs::Programs(EventView *_eventview, QListView *_programs,
	              QListView *_events)
{
	if (_eventview)
		eventview=_eventview;
	if (_programs)
		programs=_programs;
	if (_events)
		events=_events;
	
	QStringList dirs("eventsrc"); // load system-wide eventsrc
	{
		QStringList fullpaths(KGlobal::dirs()->findAllResources("data", "*/eventsrc", false, true));
		for (QStringList::Iterator it=fullpaths.begin(); it!=fullpaths.end(); ++it)
		{
			QString s=Programs::getFileWithOnlyOneSlash(*it);
			dirs+=s;
		}
	}
	
	for (QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it)
	{
		KConfig conf(*it, false, false, (*it == "eventsrc") ? "config" : "data");
		conf.setGroup("!Global!");
		ProgramConfig *prog=new ProgramConfig;
		programlist.append(prog);
		prog->configfile=*it;
		prog->load(conf);
	}
	connect(Programs::programs, SIGNAL(selectionChanged(QListViewItem*)), SLOT(selected(QListViewItem*)));
}

Programs::~Programs()
{
	programlist.setAutoDelete(true);
}

void Programs::show()
{
	// Unload what we have now
	
	// Load the new goods.

	// Put them in the app list
	for (ProgramConfig *prog=programlist.first(); prog != 0; prog=programlist.next())
		new ProgramConfig::ProgramListViewItem(prog);
	
	Programs::programs->setSelected(Programs::programs->firstChild(),true);
}

void ProgramConfig::show()
{
	// Unload the old events
	
	// and show the new ones
	for (EventConfig *ev=eventlist.first(); ev != 0; ev=eventlist.next())
		new EventConfig::EventListViewItem(ev);
	
	Programs::events->setSelected(Programs::events->firstChild(),true);
}

ProgramConfig::ProgramListViewItem::ProgramListViewItem(ProgramConfig *prog)
	: QListViewItem(Programs::programs, prog->appname, prog->description),
	  program(prog)
{
	
}

EventConfig::EventListViewItem::EventListViewItem(EventConfig *ev)
	: QListViewItem(Programs::events, ev->friendly, ev->description),
	  event(ev)
{}

void Programs::selected(QListViewItem *_i)
{
	(static_cast<ProgramConfig::ProgramListViewItem*>(_i))->program->selected();
}

void ProgramConfig::selected()
{
	// Clean up after the previous ProgramConfig
	Programs::eventview->unload();
	Programs::events->clear();
	
	// Load the new events
	for (EventConfig *ev=eventlist.first(); ev != 0; ev=eventlist.next())
		new EventConfig::EventListViewItem(ev);
		
	Programs::events->setCurrentItem(Programs::events->firstChild()); // Select the first one in the list

}

void ProgramConfig::selected(QListViewItem *_i)
{
	(static_cast<EventConfig::EventListViewItem*>(_i))->event->selected();
}

void EventConfig::selected()
{
	// Load the new events
	Programs::eventview->load(this);
}

ProgramConfig::ProgramConfig()
{
	connect(Programs::events, SIGNAL(selectionChanged(QListViewItem*)), SLOT(selected(QListViewItem*)));
}

QString Programs::getFileWithOnlyOneSlash(const QString &path)
{
	int pos=path.findRev('/');
	pos=path.findRev('/', pos-1);
	return path.right(path.length()-pos-1);
}

void EventConfig::save(KConfig &conf)
{
	conf.setGroup(internalname);
	conf.writeEntry("presentation", present);
	conf.writeEntry("soundfile", soundfile);
	conf.writeEntry("logfile",logfile);
}

void ProgramConfig::save()
{
	KConfig conf(configfile, false, false, (configfile == "eventsrc") ? "config" : "data");

	for (EventConfig *ev=eventlist.first(); ev != 0; ev=eventlist.next())
		ev->save(conf);
	
	conf.sync();
}

void Programs::save()
{
	for (ProgramConfig *prog=programlist.first(); prog != 0; prog=programlist.next())
		prog->save();
}
