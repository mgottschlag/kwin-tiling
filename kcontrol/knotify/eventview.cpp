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
#include <qheader.h>
#include <qlistview.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kaudioplayer.h>


QString iSound(i18n("Sound"));
QString iMessageBox(i18n("Message Box"));
QString iLogFile(i18n("Log File"));
QString iStandardError(i18n("Standard Error"));

EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name), event(0), oldListItem(0)
{
	QGridLayout *layout=new QGridLayout(this,4, 2,
	                                    KDialog::marginHint(),
	                                    KDialog::spacingHint());
	QHBoxLayout *tinylayout;
	layout->addLayout(tinylayout=new QHBoxLayout(this),2,1);

	
	eventslist=new QListView(this);
	eventslist->addColumn("Column");
	eventslist->header()->hide();
	
	layout->addMultiCellWidget(eventslist, 0,3, 0,0);
	tinylayout->addWidget(file=new KURLRequester(this));
	tinylayout->addWidget(play=new QPushButton(i18n("&Play"), this));
	layout->addWidget(new QLabel(file, i18n("&Filename:"), this), 1,1);
	layout->addWidget(todefault=new QPushButton(i18n("Set To &Default"), this), 3,1, Qt::AlignRight);
	
	file->setEnabled(false);
	connect(eventslist, SIGNAL(selectionChanged(QListViewItem*)), SLOT(itemSelected(QListViewItem*)));
	connect(eventslist, SIGNAL(clicked(QListViewItem*)), SLOT(itemClicked(QListViewItem*)));
	
	connect(file, SIGNAL(textChanged(const QString&)), SLOT(textChanged(const QString&)) );
	connect(todefault, SIGNAL(clicked()), SLOT(defaults()));
	connect(play, SIGNAL(clicked()), SLOT(playSound()));
};

EventView::~EventView()
{
}

void EventView::defaults()
{
	if (!KMessageBox::Yes==KMessageBox::questionYesNo(this, i18n("This will reset this entire notification to the system-default settings.\nAre you sure you want to continue?"), i18n("Continue?")))
		return;
	QCheckListItem *current=(QCheckListItem*)eventslist->currentItem();
	
	emit changed();
	event->reload();
	load(event, false);
	eventslist->setCurrentItem(current);
	
}

void EventView::textChanged(const QString &str)
{
	QCheckListItem *item=(QCheckListItem*)eventslist->selectedItem();
	if (event && item)
		if (enumNum(item) == KNotifyClient::Sound)
			event->soundfile=str;
		else if (enumNum(item) == KNotifyClient::Logfile)
			event->logfile=str;
	emit changed();
}

void EventView::itemSelected(QListViewItem *lvi)
{
        if ( !lvi || !event )
	        return;
    
        QCheckListItem *item = static_cast<QCheckListItem *>(lvi);
    
        int type = enumNum(item);
	file->setEnabled(item->isOn() && 
			 (type == KNotifyClient::Sound || type == KNotifyClient::Logfile));
	
	if (type == KNotifyClient::Sound)
		setFileURL(event->soundfile), play->show();
	else
		play->hide();
		
	if (type == KNotifyClient::Logfile)
		setFileURL(event->logfile);
	
	oldListItem=(QCheckListItem*)item;
}

void EventView::playSound()
{
	KAudioPlayer::play(file->url());
}

void EventView::itemClicked(QListViewItem* lvi)
{
        if ( !lvi )
	        return;
    
        QCheckListItem *item = static_cast<QCheckListItem *>(lvi);
    
        int type = enumNum(item);
	file->setEnabled(item->isOn() && 
			 (type == KNotifyClient::Sound || type == KNotifyClient::Logfile));

	// only emit changed(), when the checked state changed
	if ( ((event->present & type) == 0) == item->isOn() )
	        emit changed();
		
	if (item->isOn())
	        event->present|=type;
	else
	        event->present&=~type;
}

void EventView::load(EventConfig *_event, bool save)
{
        if ( !_event )
	        return;
    
	unload(save);
	
	// I load the _event
	QCheckListItem *item = 0L;
	
	// Handle the nopresent thing
	if (!(_event->nopresent & KNotifyClient::Sound)) {
		item = new QCheckListItem(eventslist, iSound, QCheckListItem::CheckBox);
		item->setOn( _event->present & KNotifyClient::Sound );
	}
	if (!(_event->nopresent & KNotifyClient::Messagebox)) {
		item = new QCheckListItem(eventslist, iMessageBox, QCheckListItem::CheckBox);
		item->setOn( _event->present & KNotifyClient::Messagebox );
	}
	if (!(_event->nopresent & KNotifyClient::Logfile)) {
		item = new QCheckListItem(eventslist, iLogFile, QCheckListItem::CheckBox);
		item->setOn( _event->present & KNotifyClient::Logfile );
	}
	if (!(_event->nopresent & KNotifyClient::Stderr)) {
		item = new QCheckListItem(eventslist, iStandardError, QCheckListItem::CheckBox);
		item->setOn( _event->present & KNotifyClient::Stderr );
	}
	
	event = _event;
	setEnabled(true);
	eventslist->setSelected(0, true);
	kapp->processEvents();
	eventslist->setContentsPos(0,0); // go to the top
	itemSelected(eventslist->firstChild());
}

void EventView::unload(bool)
{
	eventslist->clear();

	event=0;
	
	setFileURL("");
	file->setEnabled(false);
}

int EventView::enumNum(QListViewItem *item)
{
        if ( !item )
	        return 0;
    
	QString temp=item->text(0);
	if (temp==iSound) return 1;
	if (temp==iMessageBox) return 2;
	if (temp==iLogFile) return 4;
	if (temp==iStandardError) return 8;
	return 0;
}

// we get a textChanged() signal when we call setURL(), but we don't want it
void EventView::setFileURL(const QString& url)
{
        file->blockSignals(true);
	file->setURL( url );
	file->blockSignals(false);
}
