/*
    $Id$

    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qvgroupbox.h>

#include <kaboutdata.h>
#include <kapp.h>
#include <kaudioplayer.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kurlrequester.h>

#include "knotify.h"
#include "knotify.moc"

static const int COL_FILENAME = 1;

KNotifyWidget::KNotifyWidget(QWidget *parent, const char *name):
    KCModule(parent, name)
{
    currentItem = 0L;

    QVBoxLayout *lay = new QVBoxLayout( this, KDialog::marginHint(),
					KDialog::spacingHint() );
    QVGroupBox *box = new QVGroupBox( i18n("System notification settings"),
				      this );
    lay->addWidget( box );
    view =  new QListView( box );
    view->addColumn(i18n("Application/Events"));
    view->addColumn(i18n("Filename"));
    view->setSelectionMode( QListView::Single );
    view->setRootIsDecorated( true );
    view->setSorting( -1 );

    QHBox *hbox = new QHBox( box );
    hbox->setSpacing( KDialog::spacingHint() );
    QLabel *l = new QLabel( i18n("&Filename: "), hbox );
    requester = new KURLRequester( hbox );
    requester->setEnabled( false );
    l->setBuddy( requester );

    playButton = new QPushButton(  hbox );
    playButton->setFixedSize( requester->button()->size() );
    playButton->setPixmap( UserIcon("play") );
    playButton->hide();

    connect( playButton, SIGNAL( clicked() ), SLOT( playSound() ));
    connect(requester, SIGNAL( textChanged( const QString& )),
	    SLOT( slotFileChanged( const QString& )) );
    connect( view, SIGNAL( currentChanged( QListViewItem * )),
	     SLOT( slotItemActivated( QListViewItem * )));

    m_events = new Events();
    m_events->load();

    updateView();
};

KNotifyWidget::~KNotifyWidget()
{
    delete m_events;
}

/**
 * Clears the view and iterates over all apps, creating listview-items
 */
void KNotifyWidget::updateView()
{
    view->clear();
    QListViewItem *appItem = 0L;
    KNListViewItem *eItem  = 0L;
    KNEvent *e;

    QPixmap icon = SmallIcon("idea");

    // using the last appItem and eItem as "after-item" to get proper sorting
    KNApplicationListIterator it( m_events->apps() );
    while ( it.current() ) {
	appItem = new QListViewItem( view, appItem, (*it)->text() );
	appItem->setPixmap( 0, SmallIcon( (*it)->icon() ));

	// FIXME: delay that?
	KNEventListIterator it2( *(*it)->eventList() );
	while( (e = it2.current()) ) {
	    eItem = new KNListViewItem( appItem, eItem, e );
	    eItem->setPixmap( 0, icon );

	    connect( eItem, SIGNAL( changed() ), SLOT( changed() ));
	    ++it2;
	}

	++it;
    }
}


void KNotifyWidget::defaults()
{
    if (KMessageBox::warningContinueCancel(this,
        i18n("This will cause the notifications for *All Applications* "
             "to be reset to their defaults!"), i18n("Are you sure?!"), i18n("Continue"))
        != KMessageBox::Continue)
        return;

    loadAll();
}

void KNotifyWidget::changed()
{
    emit KCModule::changed(true);
}

/**
 * Someone typing in the url-requester -> update the listview item and its
 * event.
 */
void KNotifyWidget::slotFileChanged( const QString& text )
{
    if ( !currentItem ) // should never happen
	return;

    KNEvent *event = currentItem->event;
    QString *itemText = 0L;

    if ( currentItem->type == KNotifyClient::Sound )
	itemText = &(event->soundfile);
    else if ( currentItem->type == KNotifyClient::Logfile )
	itemText = &(event->logfile);

    if ( itemText && *itemText != text ) {
	*itemText = text;
	emit changed();
    }

    currentItem->setText( COL_FILENAME, text );
}

void KNotifyWidget::playSound()
{
    KAudioPlayer::play( requester->url() );
}

void KNotifyWidget::loadAll()
{
    m_events->load();
    updateView();
}

void KNotifyWidget::save()
{
    m_events->save();
    if ( !kapp->dcopClient()->isAttached() )
	kapp->dcopClient()->attach();
    kapp->dcopClient()->send("knotify", "", "reconfigure()", "");
}

void KNotifyWidget::slotItemActivated( QListViewItem *i )
{
    bool enableButton = false;
    KNCheckListItem *item = dynamic_cast<KNCheckListItem *>( i );
    if ( item ) {
	currentItem = item;
	const KNEvent *event = item->event;

	if ( item->type == KNotifyClient::Sound ) {
	    requester->setURL( event->soundfile );
	    enableButton = true;
	    playButton->show();
	}
	else if ( item->type == KNotifyClient::Logfile ) {
	    requester->setURL( event->logfile );
	    enableButton = true;
	    playButton->hide();
	}
	else {
	    requester->lineEdit()->clear();
	    playButton->hide();
	}
    }
    else {
	requester->lineEdit()->clear();
	playButton->hide();
	currentItem = 0L;
    }

    requester->setEnabled( enableButton );
}


QString KNotifyWidget::quickHelp() const
{
    return i18n("<h1>System Notifications</h1>"
		"KDE allows for a great deal of control over how you "
		"will be notified when certain events occur.  There are "
		"several choices as to how you are notified:"
		"<ul><li>As the application was originally designed."
		"<li>With a beep or other noise."
		"<li>Via a popup dialog box with additional information."
		"<li>By recording the the event in a logfile without "
		"any additional visual or auditory alert."
		"</ul>");
}

const KAboutData *KNotifyWidget::aboutData() const
{
    static KAboutData* ab = 0;

    if(!ab)
    {
        ab = new KAboutData(
            "kcmnotify", I18N_NOOP("KNotify"), "2.0pre",
            I18N_NOOP("System Notification Control Panel Module"),
            KAboutData::License_GPL, 0, 0, 0 );
        ab->addAuthor( "Carsten Pfeiffer", 0, "pfeiffer@kde.org" );
        ab->addCredit( "Charles Samuels", I18N_NOOP("Original implementation"),
                       "charles@altair.dhs.org" );
    }

    return ab;
}


///////////////////////////////////////////////////////////////////

/**
 * Custom item that represents a KNotify-event
 * creates and handles checkable child-items
 */
KNListViewItem::KNListViewItem( QListViewItem *parent,
				QListViewItem *afterItem, KNEvent *e )
    : QListViewItem( parent, afterItem, e->text() )
{
    event = e;

    if ( (e->dontShow & KNotifyClient::Stderr) == 0 ) {
	stderrItem = new KNCheckListItem( this, event, KNotifyClient::Stderr,
					  i18n("Standard error output"));
	stderrItem->setOn( e->presentation & KNotifyClient::Stderr );
    }

    if ( (e->dontShow & KNotifyClient::Messagebox) == 0 ) {
	msgboxItem = new KNCheckListItem(this, event,KNotifyClient::Messagebox,
					  i18n("Show messagebox"));
	msgboxItem->setOn( e->presentation & KNotifyClient::Messagebox );
    }

    if ( (e->dontShow & KNotifyClient::Sound) == 0 ) {
	soundItem = new KNCheckListItem( this, event, KNotifyClient::Sound,
					 i18n("Play sound"));
	soundItem->setOn( e->presentation & KNotifyClient::Sound );
	//	qDebug("******* soundfile: %s", e->soundfile.latin1() );
	soundItem->setText( COL_FILENAME, e->soundfile );
    }

    if ( (e->dontShow & KNotifyClient::Logfile) == 0 ) {
	logItem = new KNCheckListItem( this, event, KNotifyClient::Logfile,
				       i18n("Log to file"));
	logItem->setOn( e->presentation & KNotifyClient::Logfile  );
	//	qDebug("******** logfile: %s", e->logfile.latin1());
	logItem->setText( COL_FILENAME, e->logfile );
    }
}

/**
 * a child has changed -> update the KNEvent
 * not implemented as signal/slot to avoid lots of QObjects and connects
 */
void KNListViewItem::itemChanged( KNCheckListItem *item )
{
    if ( item->isOn() )
	event->presentation |= item->type;
    else
	event->presentation &= ~item->type;

    emit changed();
}



//////////////////////////////////////////////////////////////////////

/**
 * custom checkable item telling its parent when it was clicked
 */
KNCheckListItem::KNCheckListItem( QListViewItem *parent, KNEvent *e, int t,
				  const QString& text )
    : QCheckListItem( parent, text, QCheckListItem::CheckBox ),
      type( t ),
      event( e )
{
}

void KNCheckListItem::stateChange( bool )
{
    ((KNListViewItem *) parent())->itemChanged( this );
}
