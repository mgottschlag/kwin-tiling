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
#include "eventview.moc"
#include "eventconfig.h"

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kinstance.h>
#include <kaudioplayer.h>

#include <iostream.h>


QString iSound(i18n("Sound"));
QString iMessageBox("Message Box");
QString iLogFile("Log File");
QString iStandardError("Standard Error");

EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name), event(0), oldListItem(-1)
{
	QGridLayout *layout=new QGridLayout(this,4, 2, 
					    KDialog::marginHint(),
					    KDialog::spacingHint());
	QHBoxLayout *tinylayout;
	layout->addLayout(tinylayout=new QHBoxLayout(this),2,1);

	
	eventslist=new QListBox(this);
	
	layout->addMultiCellWidget(eventslist, 0,3, 0,0);
	layout->addWidget(enabled=new QCheckBox(i18n("&Enabled"),this), 0,1);
	tinylayout->addWidget(file=new KURLRequester(this));
	tinylayout->addWidget(play=new QPushButton(i18n("&Play"), this));
	layout->addWidget(new QLabel(file, i18n("&File:"), this), 1,1);
	layout->addWidget(todefault=new QPushButton(i18n("Set To &Default"), this), 3,1, Qt::AlignRight);
	
	file->setEnabled(false);
	connect(eventslist, SIGNAL(highlighted(int)), SLOT(itemSelected(int)));
	connect(enabled, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
	connect((QObject*)file->lineEdit(), SIGNAL(textChanged(const QString&)), SLOT(textChanged(const QString&)) );
	connect(todefault, SIGNAL(clicked()), SLOT(defaults()));
	connect(play, SIGNAL(clicked()), SLOT(playSound()));
};

EventView::~EventView()
{

}

void EventView::defaults()
{
	int current=eventslist->currentItem();
	
	emit changed();
	event->reload();
	load(event, false);
	eventslist->setCurrentItem(current);
}

void EventView::textChanged(const QString &str)
{
	int item=eventslist->currentItem();
	if (event)
		if (enumNum(item) == KNotifyClient::Sound)
			event->soundfile=str;
		else if (enumNum(item) == KNotifyClient::Logfile)
			event->logfile=str;
}

void EventView::itemSelected(int item)
{
	file->setEnabled(false);
	// charger la nouvelle chose
	
	enabled->setChecked((event->present & enumNum(item)) ? true : false);
	if (enumNum(item) == KNotifyClient::Sound)
		file->setEnabled(true), file->setURL(event->soundfile), play->show();
	else
		play->hide();
		
	if (enumNum(item) == KNotifyClient::Logfile)
		file->setEnabled(true), file->setURL(event->logfile);
	
		
	oldListItem=item;
}

void EventView::playSound()
{
	KAudioPlayer::play(file->url());
}

void EventView::itemToggled(bool on)
{
	if (!event) return;
	if (on)
		event->present|=enumNum(eventslist->currentItem());
	else
		event->present&= ~enumNum(eventslist->currentItem());
	setPixmaps();
}

void EventView::load(EventConfig *_event, bool save)
{
	unload(save);
	
	// I load the _event (e.g., showing all the cute little flags)
	
	// Handle the nopresent thing
	QStringList presentation;
	if (!(_event->nopresent && KNotifyClient::Sound))
		presentation << iSound;
	if (!(_event->nopresent && KNotifyClient::Messagebox))
		presentation << iMessageBox;
	if (!(_event->nopresent && KNotifyClient::Logfile))
		presentation << iLogFile;
	if (!(_event->nopresent && KNotifyClient::Stderr))
		presentation << iStandardError;
	eventslist->insertStringList(presentation);
	
	event=_event;
	setEnabled(true);
	setPixmaps();
	eventslist->setSelected(0, true);
	kapp->processEvents();
	eventslist->setContentsPos(0,0); // go to the top
	itemSelected(0);
}

void EventView::setPixmaps()
{ // Handle all of 'dem at once
	int current=eventslist->currentItem();
	eventslist->blockSignals(true);
	if (!event) return;
	int i=0;
	for (int c=1; c <=8; c*=2)
	{
		if (event->present & c)
			eventslist->changeItem(SmallIcon("flag"), eventslist->text(i), i);
		else
			eventslist->changeItem(eventslist->text(i), i);
		i++;
	}
	eventslist->setCurrentItem(current);
	eventslist->blockSignals(false);
}

void EventView::unload(bool)
{
	eventslist->clear();

	event=0;
	enabled->setChecked(false);
	setPixmaps();
	
	
	file->setURL("");
	file->setEnabled(false);
}

int EventView::enumNum(int listNum)
{
	QString temp=eventslist->text(listNum);
	if (temp==iSound) return 1;
	if (temp==iMessageBox) return 2;
	if (temp==iLogFile) return 4;
	if (temp==iStandardError) return 8;
	return 0;
}
