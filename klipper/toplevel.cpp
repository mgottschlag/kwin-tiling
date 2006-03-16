// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project

   Copyright (C) by Andrew Stanley-Jones
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qclipboard.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qmime.h>
#include <q3dragobject.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <Q3PtrList>
#include <QMouseEvent>
#include <QX11Info>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kipc.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <ksystemtray.h>
#include <kurldrag.h>
#include <kwin.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <dcopclient.h>
#include <kiconloader.h>
#include <khelpmenu.h>
#include <kstdguiitem.h>

#include "configdialog.h"
#include "toplevel.h"
#include "urlgrabber.h"
#include "version.h"
#include "clipboardpoll.h"
#include "history.h"
#include "historyitem.h"
#include "historystringitem.h"
#include "klipperpopup.h"

#include <zlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <kclipboard.h>

namespace {
    /**
     * Use this when manipulating the clipboard
     * from within clipboard-related signals.
     *
     * This avoids issues such as mouse-selections that immediately
     * disappear.
     * pattern: Resource Acqusition is Initialisation (RAII)
     *
     * (This is not threadsafe, so don't try to use such in threaded
     * applications).
     */
    struct Ignore {
        Ignore(int& locklevel) : locklevelref(locklevel)  {
            locklevelref++;
        }
        ~Ignore() {
            locklevelref--;
        }
    private:
        int& locklevelref;
    };
}

/**
 * Helper class to save history upon session exit.
 */
class KlipperSessionManaged : public KSessionManaged
{
public:
    KlipperSessionManaged( KlipperWidget* k )
        : klipper( k )
        {}

    /**
     * Save state upon session exit.
     *
     * Saving history on session save
     */
    virtual bool commitData( QSessionManager& ) {
        klipper->saveSession();
        return true;
    }
private:
    KlipperWidget* klipper;
};

extern bool qt_qclipboard_bailout_hack;
#if KDE_IS_VERSION( 3, 9, 0 )
#warning Check status of #80072 with Qt4.
#endif

static void ensureGlobalSyncOff(KConfig* config);

// config == KGlobal::config for process, otherwise applet
KlipperWidget::KlipperWidget( QWidget *parent, KConfig* config )
    : QWidget( parent )
    , DCOPObject( "klipper" )
    , m_overflowCounter( 0 )
    , locklevel( 0 )
    , m_config( config )
    , m_pendingContentsCheck( false )
    , session_managed( new KlipperSessionManaged( this ))
{
    qt_qclipboard_bailout_hack = true;

    // We don't use the clipboardsynchronizer anymore, and it confuses Klipper
    ensureGlobalSyncOff(m_config);

    updateTimestamp(); // read initial X user time
    setBackgroundMode( Qt::X11ParentRelative );
    clip = kapp->clipboard();

    connect( &m_overflowClearTimer, SIGNAL( timeout()), SLOT( slotClearOverflow()));
    m_overflowClearTimer.start( 1000 );
    connect( &m_pendingCheckTimer, SIGNAL( timeout()), SLOT( slotCheckPending()));

    m_history = new History( this, "main_history" );

    // we need that collection, otherwise KToggleAction is not happy :}
    QString defaultGroup( "default" );
    KActionCollection *collection = new KActionCollection( this );
    toggleURLGrabAction = new KToggleAction( collection, "toggleUrlGrabAction" );
    toggleURLGrabAction->setEnabled( true );
    toggleURLGrabAction->setGroup( defaultGroup );
    clearHistoryAction = new KAction( i18n("C&lear Clipboard History"),
                                      "history_clear",
                                      0,
                                      history(),
                                      SLOT( slotClear() ),
                                      collection,
                                      "clearHistoryAction" );
    connect( clearHistoryAction, SIGNAL( activated() ), SLOT( slotClearClipboard() ) );
    clearHistoryAction->setGroup( defaultGroup );
    configureAction = new KAction( i18n("&Configure Klipper..."),
                                   "configure",
                                   0,
                                   this,
                                   SLOT( slotConfigure() ),
                                   collection,
                                   "configureAction" );
    configureAction->setGroup( defaultGroup );
    quitAction = new KAction( i18n("&Quit"),
                              "exit",
                              0,
                              this,
                              SLOT( slotQuit() ),
                              collection,
                              "quitAction" );
    quitAction->setGroup( "exit" );
    myURLGrabber = 0L;
    KConfig *kc = m_config;
    readConfiguration( kc );
    setURLGrabberEnabled( bURLGrabber );

    hideTimer = new QTime();
    showTimer = new QTime();

    readProperties(m_config);
    connect(kapp, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));

    poll = new ClipboardPoll( this );
    connect( poll, SIGNAL( clipboardChanged( bool ) ),
             this, SLOT( newClipData( bool ) ) );

    m_pixmap = KSystemTray::loadIcon( "klipper" );
    adjustSize();

    globalKeys = new KGlobalAccel(this);
    KGlobalAccel* keys = globalKeys;
#include "klipperbindings.cpp"
    // the keys need to be read from kdeglobals, not kickerrc --ellis, 22/9/02
    globalKeys->readSettings();
    globalKeys->updateConnections();
    toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

    connect( toggleURLGrabAction, SIGNAL( toggled( bool )),
             this, SLOT( setURLGrabberEnabled( bool )));

    KlipperPopup* popup = history()->popup();
    connect ( history(),  SIGNAL( topChanged() ), SLOT( slotHistoryTopChanged() ) );
    connect( popup, SIGNAL( aboutToHide() ), SLOT( slotStartHideTimer() ) );
    connect( popup, SIGNAL( aboutToShow() ), SLOT( slotStartShowTimer() ) );

    popup->plugAction( toggleURLGrabAction );
    popup->plugAction( clearHistoryAction );
    popup->plugAction( configureAction );
    if ( !isApplet() ) {
        popup->plugAction( quitAction );
    }

    this->setToolTip( i18n("Klipper - clipboard tool") );
}

KlipperWidget::~KlipperWidget()
{
    delete session_managed;
    delete showTimer;
    delete hideTimer;
    delete myURLGrabber;
    if( m_config != KGlobal::config())
        delete m_config;
    qt_qclipboard_bailout_hack = false;
}

void KlipperWidget::adjustSize()
{
    resize( m_pixmap.size() );
}

// DCOP
QString KlipperWidget::getClipboardContents()
{
    return getClipboardHistoryItem(0);
}

// DCOP - don't call from Klipper itself
void KlipperWidget::setClipboardContents(QString s)
{
    Ignore lock( locklevel );
    updateTimestamp();
    HistoryStringItem* item = new HistoryStringItem( s );
    setClipboard( *item, Clipboard | Selection);
    history()->insert( item );
}

// DCOP - don't call from Klipper itself
void KlipperWidget::clearClipboardContents()
{
    updateTimestamp();
    slotClearClipboard();
}

// DCOP - don't call from Klipper itself
void KlipperWidget::clearClipboardHistory()
{
    updateTimestamp();
    slotClearClipboard();
    history()->slotClear();
    saveSession();
}


void KlipperWidget::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() != Qt::LeftButton && e->button() != Qt::RightButton )
        return;

    // if we only hid the menu less than a third of a second ago,
    // it's probably because the user clicked on the klipper icon
    // to hide it, and therefore won't want it shown again.
    if ( hideTimer->elapsed() > 300 ) {
        slotPopupMenu();
    }
}

void KlipperWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int x = (width() - m_pixmap.width()) / 2;
    int y = (height() - m_pixmap.height()) / 2;
    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    p.drawPixmap(x , y, m_pixmap);
    p.end();
}

void KlipperWidget::slotStartHideTimer()
{
    hideTimer->start();
}

void KlipperWidget::slotStartShowTimer()
{
    showTimer->start();
}

void KlipperWidget::showPopupMenu( QMenu *menu )
{
    Q_ASSERT( menu != 0L );

    QSize size = menu->sizeHint(); // geometry is not valid until it's shown
    if (bPopupAtMouse) {
        QPoint g = QCursor::pos();
        if ( size.height() < g.y() )
            menu->popup(QPoint( g.x(), g.y() - size.height()));
        else
            menu->popup(QPoint(g.x(), g.y()));
    } else {
        KWin::WindowInfo i = KWin::windowInfo( winId(), NET::WMGeometry );
        QRect g = i.geometry();
        QRect screen = KGlobalSettings::desktopGeometry(g.center());

        if ( g.x()-screen.x() > screen.width()/2 &&
             g.y()-screen.y() + size.height() > screen.height() )
            menu->popup(QPoint( g.x(), g.y() - size.height()));
        else
            menu->popup(QPoint( g.x() + width(), g.y() + height()));

        //      menu->exec(mapToGlobal(QPoint( width()/2, height()/2 )));
    }
}

bool KlipperWidget::loadHistory() {
    static const char* const failed_load_warning =
        "Failed to load history resource. Clipboard history cannot be read.";
    // don't use "appdata", klipper is also a kicker applet
    QString history_file_name = ::locateLocal( "data", "klipper/history2.lst" );
    QFile history_file( history_file_name );
    bool oldfile = false;
    if ( !history_file.exists() ) { // backwards compatibility
        oldfile = true;
        history_file_name = ::locateLocal( "data", "klipper/history.lst" );
        history_file.setName( history_file_name );
        if ( !history_file.exists() ) {
            history_file_name = ::locateLocal( "data", "kicker/history.lst" );
            history_file.setName( history_file_name );
            if ( !history_file.exists() ) {
                return false;
            }
        }
    }
    if ( !history_file.open( QIODevice::ReadOnly ) ) {
        kWarning() << failed_load_warning << ": " << history_file.errorString() << endl;
        return false;
    }
    QDataStream file_stream( &history_file );
    if( file_stream.atEnd()) {
        kWarning() << failed_load_warning << endl;
        return false;
    }
    QDataStream* history_stream = &file_stream;
    QByteArray data;
    if( !oldfile ) {
        Q_UINT32 crc;
        file_stream >> crc >> data;
        if( crc32( 0, reinterpret_cast<unsigned char *>( data.data() ), data.size() ) != crc ) {
            kWarning() << failed_load_warning << ": " << history_file.errorString() << endl;
            return false;
        }
        history_stream = new QDataStream( &data, QIODevice::ReadOnly );
    }
    char* version;
    *history_stream >> version;
    delete[] version;

    // The list needs to be reversed, as it is saved
    // youngest-first to keep the most important clipboard
    // items at the top, but the history is created oldest
    // first.
    Q3PtrList<HistoryItem> reverseList;
    for ( HistoryItem* item = HistoryItem::create( *history_stream );
          item;
          item = HistoryItem::create( *history_stream ) )
    {
        reverseList.prepend( item );
    }

    for ( HistoryItem* item = reverseList.first();
          item;
          item = reverseList.next() )
    {
        history()->forceInsert( item );
    }

    if ( !history()->empty() ) {
        m_lastSelection = -1;
        m_lastClipboard = -1;
        setClipboard( *history()->first(), Clipboard | Selection );
    }

    if( history_stream != &file_stream )
        delete history_stream;

    return true;
}

void KlipperWidget::saveHistory() {
    static const char* const failed_save_warning =
        "Failed to save history. Clipboard history cannot be saved.";
    // don't use "appdata", klipper is also a kicker applet
    QString history_file_name( ::locateLocal( "data", "klipper/history2.lst" ) );
    if ( history_file_name.isNull() || history_file_name.isEmpty() ) {
        kWarning() << failed_save_warning << endl;
        return;
    }
    KSaveFile history_file( history_file_name );
    if ( history_file.status() != 0  ) {
        kWarning() << failed_save_warning << endl;
        return;
    }
    QByteArray data;
    QDataStream history_stream( &data, QIODevice::WriteOnly );
    history_stream << klipper_version; // const char*
    for (  const HistoryItem* item = history()->first(); item; item = history()->next() ) {
        history_stream << item;
    }
    Q_UINT32 crc = crc32( 0, reinterpret_cast<unsigned char *>( data.data() ), data.size() );
    *history_file.dataStream() << crc << data;
}

void KlipperWidget::readProperties(KConfig *kc)
{
    QStringList dataList;

    history()->slotClear();

    if (bKeepContents) { // load old clipboard if configured
        if ( !loadHistory() ) {
            // Try to load from the old config file.
            // Remove this at some point.
            KConfigGroup configGroup(kc, "General");
            dataList = configGroup.readEntry("ClipboardData",QStringList());

            for (QStringList::ConstIterator it = dataList.end();
                 it != dataList.begin();
             )
            {
                history()->forceInsert( new HistoryStringItem( *( --it ) ) );
            }

            if ( !dataList.isEmpty() )
            {
                m_lastSelection = -1;
                m_lastClipboard = -1;
                setClipboard( *history()->first(), Clipboard | Selection );
            }
        }
    }

}


void KlipperWidget::readConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    bPopupAtMouse = kc->readEntry("PopupAtMousePosition", QVariant(false)).toBool();
    bKeepContents = kc->readEntry("KeepClipboardContents", QVariant(true)).toBool();
    bURLGrabber = kc->readEntry("URLGrabberEnabled", QVariant(false)).toBool();
    bReplayActionInHistory = kc->readEntry("ReplayActionInHistory", QVariant(false)).toBool();
    bNoNullClipboard = kc->readEntry("NoEmptyClipboard", QVariant(true)).toBool();
    bUseGUIRegExpEditor = kc->readEntry("UseGUIRegExpEditor", QVariant(true )).toBool();
    history()->max_size( kc->readEntry("MaxClipItems", 7) );
    bIgnoreSelection = kc->readEntry("IgnoreSelection", QVariant(false)).toBool();
    bSynchronize = kc->readEntry("Synchronize", QVariant(false)).toBool();
    bSelectionTextOnly = kc->readEntry("SelectionTextOnly",true);
    bIgnoreImages = kc->readEntry("IgnoreImages",true);
}

void KlipperWidget::writeConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    kc->writeEntry("PopupAtMousePosition", bPopupAtMouse);
    kc->writeEntry("KeepClipboardContents", bKeepContents);
    kc->writeEntry("ReplayActionInHistory", bReplayActionInHistory);
    kc->writeEntry("NoEmptyClipboard", bNoNullClipboard);
    kc->writeEntry("UseGUIRegExpEditor", bUseGUIRegExpEditor);
    kc->writeEntry("MaxClipItems", history()->max_size() );
    kc->writeEntry("IgnoreSelection", bIgnoreSelection);
    kc->writeEntry("Synchronize", bSynchronize );
    kc->writeEntry("SelectionTextOnly", bSelectionTextOnly);
    kc->writeEntry("TrackImages", bIgnoreImages);
    kc->writeEntry("Version", klipper_version );

    if ( myURLGrabber )
        myURLGrabber->writeConfiguration( kc );

    kc->sync();
}

// save session on shutdown. Don't simply use the c'tor, as that may not be called.
void KlipperWidget::saveSession()
{
    if ( bKeepContents ) { // save the clipboard eventually
        saveHistory();
    }
}

void KlipperWidget::slotSettingsChanged( int category )
{
    if ( category == (int) KApplication::SETTINGS_SHORTCUTS ) {
        globalKeys->readSettings();
        globalKeys->updateConnections();
        toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));
    }
}

void KlipperWidget::disableURLGrabber()
{
    KMessageBox::information( 0L,
                              i18n( "You can enable URL actions later by right-clicking on the "
                                    "Klipper icon and selecting 'Enable Actions'" ) );

    setURLGrabberEnabled( false );
}

void KlipperWidget::slotConfigure()
{
    bool haveURLGrabber = bURLGrabber;
    if ( !myURLGrabber ) { // temporary, for the config-dialog
        setURLGrabberEnabled( true );
        readConfiguration( m_config );
    }

    ConfigDialog *dlg = new ConfigDialog( myURLGrabber->actionList(),
                                          globalKeys, isApplet() );
    dlg->setKeepContents( bKeepContents );
    dlg->setPopupAtMousePos( bPopupAtMouse );
    dlg->setStripWhiteSpace( myURLGrabber->trimmed() );
    dlg->setReplayActionInHistory( bReplayActionInHistory );
    dlg->setNoNullClipboard( bNoNullClipboard );
    dlg->setUseGUIRegExpEditor( bUseGUIRegExpEditor );
    dlg->setPopupTimeout( myURLGrabber->popupTimeout() );
    dlg->setMaxItems( history()->max_size() );
    dlg->setIgnoreSelection( bIgnoreSelection );
    dlg->setSynchronize( bSynchronize );
    dlg->setNoActionsFor( myURLGrabber->avoidWindows() );

    if ( dlg->exec() == QDialog::Accepted ) {
        bKeepContents = dlg->keepContents();
        bPopupAtMouse = dlg->popupAtMousePos();
        bReplayActionInHistory = dlg->replayActionInHistory();
        bNoNullClipboard = dlg->noNullClipboard();
        bIgnoreSelection = dlg->ignoreSelection();
        bSynchronize = dlg->synchronize();
        bUseGUIRegExpEditor = dlg->useGUIRegExpEditor();
        dlg->commitShortcuts();
        // the keys need to be written to kdeglobals, not kickerrc --ellis, 22/9/02
        globalKeys->writeSettings(0, true);
        globalKeys->updateConnections();
        toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

        myURLGrabber->setActionList( dlg->actionList() );
        myURLGrabber->setPopupTimeout( dlg->popupTimeout() );
        myURLGrabber->setStripWhiteSpace( dlg->stripWhiteSpace() );
        myURLGrabber->setAvoidWindows( dlg->noActionsFor() );

        history()->max_size( dlg->maxItems() );

        writeConfiguration( m_config );

    }
    setURLGrabberEnabled( haveURLGrabber );

    delete dlg;
}

void KlipperWidget::slotQuit()
{
    // If the menu was just opened, likely the user
    // selected quit by accident while attempting to
    // click the Klipper icon.
    if ( showTimer->elapsed() < 300 ) {
        return;
    }

    saveSession();
    int autoStart = KMessageBox::questionYesNoCancel( 0L, i18n("Should Klipper start automatically\nwhen you login?"), i18n("Automatically Start Klipper?"), i18n("Start"), i18n("Do Not Start") );

    KConfig *config = KGlobal::config();
    config->setGroup("General");
    if ( autoStart == KMessageBox::Yes ) {
        config->writeEntry("AutoStart", true);
    } else if ( autoStart == KMessageBox::No) {
        config->writeEntry("AutoStart", false);
    } else  // cancel chosen don't quit
        return;
    config->sync();

    kapp->quit();

}

void KlipperWidget::slotPopupMenu() {
    KlipperPopup* popup = history()->popup();
    popup->ensureClean();
    showPopupMenu( popup );
}


void KlipperWidget::slotRepeatAction()
{
    if ( !myURLGrabber ) {
        myURLGrabber = new URLGrabber( m_config );
        connect( myURLGrabber, SIGNAL( sigPopup( QMenu * )),
                 SLOT( showPopupMenu( QMenu * )) );
        connect( myURLGrabber, SIGNAL( sigDisablePopup() ),
                 this, SLOT( disableURLGrabber() ) );
    }

    const HistoryStringItem* top = dynamic_cast<const HistoryStringItem*>( history()->first() );
    if ( top ) {
        myURLGrabber->invokeAction( top->text() );
    }
}

void KlipperWidget::setURLGrabberEnabled( bool enable )
{
    if (enable != bURLGrabber) {
      bURLGrabber = enable;
      KConfig *kc = m_config;
      kc->setGroup("General");
      kc->writeEntry("URLGrabberEnabled", bURLGrabber);
      m_lastURLGrabberTextSelection = QString();
      m_lastURLGrabberTextClipboard = QString();
    }

    toggleURLGrabAction->setChecked( enable );

    if ( !bURLGrabber ) {
        delete myURLGrabber;
        myURLGrabber = 0L;
        toggleURLGrabAction->setText(i18n("Enable &Actions"));
    }

    else {
        toggleURLGrabAction->setText(i18n("&Actions Enabled"));
        if ( !myURLGrabber ) {
            myURLGrabber = new URLGrabber( m_config );
            connect( myURLGrabber, SIGNAL( sigPopup( QMenu * )),
                     SLOT( showPopupMenu( QMenu * )) );
            connect( myURLGrabber, SIGNAL( sigDisablePopup() ),
                     this, SLOT( disableURLGrabber() ) );
        }
    }
}

void KlipperWidget::toggleURLGrabber()
{
    setURLGrabberEnabled( !bURLGrabber );
}

void KlipperWidget::slotHistoryTopChanged() {
    if ( locklevel ) {
        return;
    }

    const HistoryItem* topitem = history()->first();
    if ( topitem ) {
        setClipboard( *topitem, Clipboard | Selection );
    }
    if ( bReplayActionInHistory && bURLGrabber ) {
        slotRepeatAction();
    }

}

void KlipperWidget::slotClearClipboard()
{
    Ignore lock( locklevel );

    clip->clear(QClipboard::Selection);
    clip->clear(QClipboard::Clipboard);


}


//XXX: Should die, and the DCOP signal handled sensible.
QString KlipperWidget::clipboardContents( bool * /*isSelection*/ )
{
    kWarning() << "Obsolete function called. Please fix" << endl;

#if 0
    bool selection = true;
    QMimeSource* data = clip->data(QClipboard::Selection);

    if ( data->serialNumber() == m_lastSelection )
    {
        QString clipContents = clip->text(QClipboard::Clipboard);
        if ( clipContents != m_lastClipboard )
        {
            contents = clipContents;
            selection = false;
        }
        else
            selection = true;
    }

    if ( isSelection )
        *isSelection = selection;

#endif

    return 0;
}

void KlipperWidget::applyClipChanges( const QMimeSource& clipData )
{
    if ( locklevel )
        return;
    Ignore lock( locklevel );
    history()->insert( HistoryItem::create( clipData ) );

}

void KlipperWidget::newClipData( bool selectionMode )
{
    if ( locklevel ) {
        return;
    }

    if( blockFetchingNewData())
        return;

    checkClipData( selectionMode );

}

void KlipperWidget::clipboardSignalArrived( bool selectionMode )
{
    if ( locklevel ) {
        return;
    }
    if( blockFetchingNewData())
        return;

    updateTimestamp();
    checkClipData( selectionMode );
}

// Protection against too many clipboard data changes. Lyx responds to clipboard data
// requests with setting new clipboard data, so if Lyx takes over clipboard,
// Klipper notices, requests this data, this triggers "new" clipboard contents
// from Lyx, so Klipper notices again, requests this data, ... you get the idea.
const int MAX_CLIPBOARD_CHANGES = 10; // max changes per second

bool KlipperWidget::blockFetchingNewData()
{
// Hacks for #85198 and #80302.
// #85198 - block fetching new clipboard contents if Shift is pressed and mouse is not,
//   this may mean the user is doing selection using the keyboard, in which case
//   it's possible the app sets new clipboard contents after every change - Klipper's
//   history would list them all.
// #80302 - OOo (v1.1.3 at least) has a bug that if Klipper requests its clipboard contents
//   while the user is doing a selection using the mouse, OOo stops updating the clipboard
//   contents, so in practice it's like the user has selected only the part which was
//   selected when Klipper asked first.
	Qt::ButtonState buttonstate = QApplication::mouseButtons();
    if( ( buttonstate & ( Qt::ShiftModifier | Qt::LeftButton )) == Qt::ShiftButton // #85198
        || ( buttonstate & Qt::LeftButton ) == Qt::LeftButton ) { // #80302
        m_pendingContentsCheck = true;
        m_pendingCheckTimer.start( 100, true );
        return true;
    }
    m_pendingContentsCheck = false;
    if( ++m_overflowCounter > MAX_CLIPBOARD_CHANGES )
        return true;
    return false;
}

void KlipperWidget::slotCheckPending()
{
    if( !m_pendingContentsCheck )
        return;
    m_pendingContentsCheck = false; // blockFetchingNewData() will be called again
    updateTimestamp();
    newClipData( true ); // always selection
}

void KlipperWidget::checkClipData( bool selectionMode )
{
    if ( ignoreClipboardChanges() ) // internal to klipper, ignoring QSpinBox selections
    {
        // keep our old clipboard, thanks
        // This won't quite work, but it's close enough for now.
        // The trouble is that the top selection =! top clipboard
        // but we don't track that yet. We will....
        const HistoryItem* top = history()->first();
        if ( top ) {
            setClipboard( *top, selectionMode ? Selection : Clipboard);
        }
        return;
    }

// debug code

// debug code
#ifdef NOISY_KLIPPER
    kDebug() << "Checking clip data" << endl;
#endif
#if 0
    kDebug() << "====== c h e c k C l i p D a t a ============================"
              << kBacktrace()
              << "====== c h e c k C l i p D a t a ============================"
              << endl;;
#endif
#if 0
    if ( sender() ) {
        kDebug() << "sender=" << sender()->name() << endl;
    } else {
        kDebug() << "no sender" << endl;
    }
#endif
#if 0
    kDebug() << "\nselectionMode=" << selectionMode
              << "\nserialNo=" << clip->data()->serialNumber() << " (sel,cli)=(" << m_lastSelection << "," << m_lastClipboard << ")"
              << "\nowning (sel,cli)=(" << clip->ownsSelection() << "," << clip->ownsClipboard() << ")"
              << "\ntext=" << clip->text( selectionMode ? QClipboard::Selection : QClipboard::Clipboard) << endl;

#endif
#if 0
    const char *format;
    int i = 0;
    while ( (format = clip->data()->format( i++ )) )
    {
        qDebug( "    format: %s", format);
    }
#endif
    QMimeSource* data = clip->data( selectionMode ? QClipboard::Selection : QClipboard::Clipboard );
    if ( !data ) {
        kWarning("No data in clipboard. This not not supposed to happen." );
        return;
    }
    // TODO: Rewrite to Qt4 !!!
#if 0
    int lastSerialNo = selectionMode ? m_lastSelection : m_lastClipboard;
    bool changed = data->serialNumber() != lastSerialNo;
    bool clipEmpty = ( data->format() == 0L );

    if ( changed && clipEmpty && bNoNullClipboard ) {
        const HistoryItem* top = history()->first();
        if ( top ) {
            // keep old clipboard after someone set it to null
#ifdef NOISY_KLIPPER
            kDebug() << "Resetting clipboard (Prevent empty clipboard)" << endl;
#endif
            setClipboard( *top, selectionMode ? Selection : Clipboard );
        }
        return;
    }
#endif
    // this must be below the "bNoNullClipboard" handling code!
    // XXX: I want a better handling of selection/clipboard in general.
    // XXX: Order sensitive code. Must die.
    if ( selectionMode && bIgnoreSelection )
        return;
        
    if( selectionMode && bSelectionTextOnly && !Q3TextDrag::canDecode( data ))
        return;

#ifdef __GNUC__
#warning This should be maybe extended for KDE4 or at least get a checkbox somewhere in UI
#endif
    if( KURLDrag::canDecode( data ))
        ; // ok
    else if( Q3TextDrag::canDecode( data ))
        ; // ok
    else if( Q3ImageDrag::canDecode( data ))
    {
// Limit mimetypes that are tracked by Klipper (this is basically a workaround
// for #109032). Can't add UI in 3.5 because of string freeze, and I'm not sure
// if this actually needs to be more configurable than only text vs all klipper knows.
        if( bIgnoreImages )
            return;
        else
            ; // ok
    }
    else // unknown, ignore
        return;

    // store old contents:
#if 0
    if ( selectionMode )
        m_lastSelection = data->serialNumber();
    else
        m_lastClipboard = data->serialNumber();
#endif

    QString& lastURLGrabberText = selectionMode
        ? m_lastURLGrabberTextSelection : m_lastURLGrabberTextClipboard;
    if( Q3TextDrag::canDecode( data ))
    {
        if ( bURLGrabber && myURLGrabber )
        {
            QString text;
            Q3TextDrag::decode( data, text );
            // Make sure URLGrabber doesn't repeat all the time if klipper reads the same
            // text all the time (e.g. because XFixes is not available and the application
            // has broken TIMESTAMP target). Using most recent history item may not always
            // work.
            if ( text != lastURLGrabberText )
            {
                lastURLGrabberText = text;
                if ( myURLGrabber->checkNewData( text ) )
                {
                    return; // don't add into the history
                }
            }
        }
        else
            lastURLGrabberText = QString();
    }
    else
        lastURLGrabberText = QString();
#if 0
    if (changed) {
        applyClipChanges( *data );
#ifdef NOISY_KLIPPER
        kDebug() << "Synchronize?" << ( bSynchronize ? "yes" : "no" ) << endl;
#endif
        if ( bSynchronize ) {
            const HistoryItem* topItem = history()->first();
            if ( topItem ) {
                setClipboard( *topItem, selectionMode ? Clipboard : Selection );
            }
        }
    }
#endif
}

void KlipperWidget::setClipboard( const HistoryItem& item, int mode )
{
    Ignore lock( locklevel );

    Q_ASSERT( ( mode & 1 ) == 0 ); // Warn if trying to pass a boolean as a mode.

    if ( mode & Selection ) {
#ifdef NOSIY_KLIPPER
        kDebug() << "Setting selection to <" << item.text() << ">" << endl;
#endif
        clip->setData( item.mimeSource(), QClipboard::Selection );
#if 0
        m_lastSelection = clip->data()->serialNumber();<
#endif
    }
    if ( mode & Clipboard ) {
#ifdef NOSIY_KLIPPER
        kDebug() << "Setting clipboard to <" << item.text() << ">" << endl;
#endif
        clip->setData( item.mimeSource(), QClipboard::Clipboard );
#if 0
        m_lastClipboard = clip->data()->serialNumber();
#endif
    }

}

void KlipperWidget::slotClearOverflow()
{
    if( m_overflowCounter > MAX_CLIPBOARD_CHANGES ) {
        kDebug() << "App owning the clipboard/selection is lame" << endl;
        // update to the latest data - this unfortunately may trigger the problem again
        newClipData( true ); // Always the selection.
    }
    m_overflowCounter = 0;
}

QStringList KlipperWidget::getClipboardHistoryMenu()
{
    QStringList menu;
    for ( const HistoryItem* item = history()->first(); item; item = history()->next() ) {
        menu << item->text();
    }
    return menu;
}

QString KlipperWidget::getClipboardHistoryItem(int i)
{
    for ( const HistoryItem* item = history()->first(); item; item = history()->next() , i-- ) {
        if ( i == 0 ) {
            return item->text();
        }
    }
    return QString();

}

//
// changing a spinbox in klipper's config-dialog causes the lineedit-contents
// of the spinbox to be selected and hence the clipboard changes. But we don't
// want all those items in klipper's history. See #41917
//
bool KlipperWidget::ignoreClipboardChanges() const
{
    QWidget *focusWidget = qApp->focusWidget();
    if ( focusWidget )
    {
        if ( focusWidget->inherits( "QSpinBox" ) ||
             (focusWidget->parentWidget() &&
              focusWidget->inherits("QLineEdit") &&
              focusWidget->parentWidget()->inherits("QSpinWidget")) )
        {
            return true;
        }
    }

    return false;
}

// QClipboard uses qt_x_time as the timestamp for selection operations.
// It is updated mainly from user actions, but Klipper polls the clipboard
// without any user action triggering it, so qt_x_time may be old,
// which could possibly lead to QClipboard reporting empty clipboard.
// Therefore, qt_x_time needs to be updated to current X server timestamp.

// Call KApplication::updateUserTime() only from functions that are
// called from outside (DCOP), or from QTimer timeout !

static Time next_x_time;
static Bool update_x_time_predicate( Display*, XEvent* event, XPointer )
{
    if( next_x_time != CurrentTime )
        return False;
    // from qapplication_x11.cpp
    switch ( event->type ) {
    case ButtonPress:
      // fallthrough intended
    case ButtonRelease:
      next_x_time = event->xbutton.time;
      break;
    case MotionNotify:
      next_x_time = event->xmotion.time;
      break;
    case KeyPress:
      // fallthrough intended
    case KeyRelease:
      next_x_time = event->xkey.time;
      break;
    case PropertyNotify:
      next_x_time = event->xproperty.time;
      break;
    case EnterNotify:
    case LeaveNotify:
      next_x_time = event->xcrossing.time;
      break;
    case SelectionClear:
      next_x_time = event->xselectionclear.time;
      break;
    default:
      break;
    }
    return False;
}

void KlipperWidget::updateTimestamp()
{
    Time time = QX11Info::appTime();
    static QWidget* w = 0;
    if ( !w )
        w = new QWidget;
    unsigned char data[ 1 ];
    XChangeProperty( QX11Info::display(), w->winId(), XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
    next_x_time = CurrentTime;
    XEvent dummy;
    XCheckIfEvent( QX11Info::display(), &dummy, update_x_time_predicate, NULL );
    if( next_x_time == CurrentTime )
        {
        XSync( QX11Info::display(), False );
        XCheckIfEvent( QX11Info::display(), &dummy, update_x_time_predicate, NULL );
        }
    Q_ASSERT( next_x_time != CurrentTime );
    time = next_x_time;
    XEvent ev; // remove the PropertyNotify event from the events queue
    XWindowEvent( QX11Info::display(), w->winId(), PropertyChangeMask, &ev );
}

static const char * const description =
      I18N_NOOP("KDE cut & paste history utility");

void KlipperWidget::createAboutData()
{
  about_data = new KAboutData("klipper", I18N_NOOP("Klipper"),
    klipper_version, description, KAboutData::License_GPL,
		       "(c) 1998, Andrew Stanley-Jones\n"
		       "1998-2002, Carsten Pfeiffer\n"
		       "2001, Patrick Dubroy");

  about_data->addAuthor("Carsten Pfeiffer",
                      I18N_NOOP("Author"),
                      "pfeiffer@kde.org");

  about_data->addAuthor("Andrew Stanley-Jones",
                      I18N_NOOP( "Original Author" ),
                      "asj@cban.com");

  about_data->addAuthor("Patrick Dubroy",
                      I18N_NOOP("Contributor"),
                      "patrickdu@corel.com");

  about_data->addAuthor( "Luboš Luňák",
                      I18N_NOOP("Bugfixes and optimizations"),
                      "l.lunak@kde.org");

  about_data->addAuthor( "Esben Mose Hansen",
                      I18N_NOOP("Maintainer"),
                      "kde@mosehansen.dk");
}

void KlipperWidget::destroyAboutData()
{
  delete about_data;
  about_data = NULL;
}

KAboutData* KlipperWidget::about_data;

KAboutData* KlipperWidget::aboutData()
{
  return about_data;
}

Klipper::Klipper( QWidget* parent )
    : KlipperWidget( parent, KGlobal::config())
{
}

// this sucks ... KUniqueApplication registers itself as 'klipper'
// for the unique-app detection calls (and it shouldn't use that name IMHO)
// but in Klipper it's not KUniqueApplication class who handles
// the DCOP calls, but an instance of class Klipper, registered as 'klipper'
// this below avoids a warning when KUniqueApplication wouldn't otherwise
// find newInstance()  (which doesn't do anything in Klipper anyway)
int Klipper::newInstance()
{
    kapp->dcopClient()->setPriorityCall(false); // Allow other dcop calls
    return 0;
}

// this is used for quiting klipper process, if klipper is being started as an applet
// (AKA ugly hack)
void Klipper::quitProcess()
{
    kapp->dcopClient()->detach();
    kapp->quit();
}

static void ensureGlobalSyncOff(KConfig* config) {
    config->setGroup("General");
    if ( config->readEntry( "SynchronizeClipboardAndSelection" , QVariant(false)).toBool() ) {
        kDebug() << "Shutting off global synchronization" << endl;
        config->writeEntry("SynchronizeClipboardAndSelection", false, KConfig::Normal | KConfig::Global );
        config->sync();
        KClipboardSynchronizer::setSynchronizing( false );
        KClipboardSynchronizer::setReverseSynchronizing( false );
        KIPC::sendMessageAll( KIPC::ClipboardConfigChanged, 0 );
    }

}

#include "toplevel.moc"
