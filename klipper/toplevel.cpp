/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */

#include <qclipboard.h>
#include <qcursor.h>
#include <qfile.h>
#include <qintdict.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kclipboard.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kipc.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <ksystemtray.h>
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

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define QUIT_ITEM     50
#define CONFIG_ITEM   60
#define EMPTY_ITEM    80
#define HELPMENU_ITEM 90
#define TITLE_ITEM    100

#define MENU_ITEMS   (( isApplet() ? 6 : 8 ) + ( bTearOffHandle ? 1 : 0 ))
// the <clipboard empty> item
#define EMPTY (m_popup->count() - MENU_ITEMS)

extern bool qt_qclipboard_bailout_hack;

// config == kapp->config for process, otherwise applet
KlipperWidget::KlipperWidget( QWidget *parent, KConfig* config )
    : QWidget( parent ), DCOPObject( "klipper" ), m_config( config )
{
    qt_qclipboard_bailout_hack = true;
    updateTimestamp(); // read initial X user time
    setBackgroundMode( X11ParentRelative );
    clip = kapp->clipboard();
    m_selectedItem = -1;

    QSempty = i18n("<empty clipboard>");

    bTearOffHandle = KGlobalSettings::insertTearOffHandle();

    // we need that collection, otherwise KToggleAction is not happy :}
    KActionCollection *collection = new KActionCollection( this, "my collection" );
    toggleURLGrabAction = new KToggleAction( collection, "toggleUrlGrabAction" );
    toggleURLGrabAction->setEnabled( true );

    myURLGrabber = 0L;
    KConfig *kc = m_config;
    readConfiguration( kc );
    setURLGrabberEnabled( bURLGrabber );

    m_lastString = "";
    m_popup = new KPopupMenu(0L, "main_menu");
    connect(m_popup, SIGNAL(activated(int)), SLOT(clickedMenu(int)));

    readProperties(m_config);
    connect(kapp, SIGNAL(saveYourself()), SLOT(saveSession()));
    connect(kapp, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));

    poll = new ClipboardPoll( this );
    connect( poll, SIGNAL( clipboardChanged()), this, SLOT(newClipData()));
    connect( clip, SIGNAL( selectionChanged() ), SLOT(slotSelectionChanged()));
    connect( clip, SIGNAL( dataChanged() ), SLOT( slotClipboardChanged() ));

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

    QToolTip::add( this, i18n("Klipper - clipboard tool") );
}

KlipperWidget::~KlipperWidget()
{
    delete m_popup;
    delete myURLGrabber;
    if( m_config != kapp->config())
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
    return clipboardContents();
}

// DCOP - don't call from Klipper itself
void KlipperWidget::setClipboardContents(QString s)
{
    updateTimestamp();
    setClipboard( s, Clipboard | Selection);
    newClipData();
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
  trimClipHistory(0);
  saveSession();
}


void KlipperWidget::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() != LeftButton && e->button() != RightButton )
        return;

    showPopupMenu( m_popup );
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

void KlipperWidget::clickedMenu(int id)
{
    switch ( id ) {
    case -1:
        break;
    case CONFIG_ITEM:
        slotConfigure();
        break;
    case QUIT_ITEM: {
        saveSession();
        int autoStart = KMessageBox::questionYesNoCancel( 0L, i18n("Should Klipper start automatically\nwhen you login?"), i18n("Automatically Start Klipper?") );

        KConfig *config = KGlobal::config();
        config->setGroup("General");
        if ( autoStart == KMessageBox::Yes )
            config->writeEntry("AutoStart", true);
        else if ( autoStart == KMessageBox::No) {
            config->writeEntry("AutoStart", false);
        } else  // cancel chosen don't quit
            break;
        config->sync();
        kapp->quit();
        break;
    }
//    case URLGRAB_ITEM: // handled with an extra slot
//	break;
    case EMPTY_ITEM:
	if ( !bClipEmpty )
	{
        trimClipHistory(0);
        slotClearClipboard();
        setEmptyClipboard();
	}
	break;
    default:
	if ( id == URLGrabItem )
	{
	    break; // handled by its own slot
	}
	else if ( !bClipEmpty )
	{
	    //CT mark up the currently put into clipboard -
            // so that user can see later
            if ( m_selectedItem != -1 )
                m_popup->setItemChecked(m_selectedItem, false);

	    m_selectedItem = id;
	    m_popup->setItemChecked(m_selectedItem, true);
            QMapIterator<long,QString> it = m_clipDict.find( id );

            if ( it != m_clipDict.end() && it.data() != QSempty )
            {
                QString data = it.data();
                setClipboard( data, Clipboard | Selection );

		if (bURLGrabber && bReplayActionInHistory)
		    myURLGrabber->checkNewData( data );

                m_lastString = data;

                // We want to move the just selected item to the top of the popup
                // menu. But when we do this right here, we get a crash a little
                // bit later. So instead, we fire a timer to perform the moving.
                QTimer::singleShot( 0, this, SLOT( slotMoveSelectedToTop() ));
            }
	}
    }
}


void KlipperWidget::showPopupMenu( QPopupMenu *menu )
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
        KWin::Info i = KWin::info( winId() );
        QRect g = i.geometry;
	QRect screen = KGlobalSettings::desktopGeometry(g.center());

        if ( g.x()-screen.x() > screen.width()/2 &&
             g.y()-screen.y() + size.height() > screen.height() )
            menu->popup(QPoint( g.x(), g.y() - size.height()));
        else
            menu->popup(QPoint( g.x() + width(), g.y() + height()));

        //      menu->exec(mapToGlobal(QPoint( width()/2, height()/2 )));
    }
}


void KlipperWidget::readProperties(KConfig *kc)
{
  QStringList dataList;

  m_popup->clear();

  m_popup->insertTitle( SmallIcon( "klipper" ),
                        i18n("Klipper - Clipboard Tool"));

  if (bKeepContents) { // load old clipboard if configured
      KConfigGroupSaver groupSaver(kc, "General");
      dataList = kc->readListEntry("ClipboardData");

      for (QStringList::ConstIterator it = dataList.begin();
           it != dataList.end(); ++it)
      {
          long id = m_popup->insertItem( KStringHandler::cEmSqueeze((*it).simplifyWhiteSpace(), fontMetrics(), 25).replace( "&", "&&" ), -2, -1);
          m_clipDict.insert( id, *it );
      }

      if ( !dataList.isEmpty() )
      {
          m_lastSelection = dataList.first();
          m_lastClipboard = dataList.first();
          m_lastString    = dataList.first();
          setClipboard( m_lastString, Clipboard | Selection );
      }
  }

  bClipEmpty = clipboardContents().simplifyWhiteSpace().isEmpty() &&
               dataList.isEmpty();

  m_popup->insertSeparator();
  toggleURLGrabAction->plug( m_popup, -1 );
  URLGrabItem = m_popup->idAt( m_popup->count() - 1 );

  m_popup->insertItem( SmallIcon("history_clear"),
			i18n("C&lear Clipboard History"), EMPTY_ITEM );

  m_popup->insertItem( SmallIcon("configure"), i18n("&Configure Klipper..."),
                        CONFIG_ITEM);

  KHelpMenu *help = new KHelpMenu( this, aboutData(),
                false );
  m_popup->insertItem( SmallIconSet("help"), KStdGuiItem::help().text(), help->menu(), HELPMENU_ITEM );

  if( !isApplet()) {
    m_popup->insertSeparator();
    m_popup->insertItem(SmallIcon("exit"), i18n("&Quit"), QUIT_ITEM );
  }
  if( bTearOffHandle )
    m_popup->insertTearOffHandle();

  if (bClipEmpty)
      setEmptyClipboard();
}


void KlipperWidget::readConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    bPopupAtMouse = kc->readBoolEntry("PopupAtMousePosition", false);
    bKeepContents = kc->readBoolEntry("KeepClipboardContents", true);
    bURLGrabber = kc->readBoolEntry("URLGrabberEnabled", false);
    bReplayActionInHistory = kc->readBoolEntry("ReplayActionInHistory", false);
    bNoNullClipboard = kc->readBoolEntry("NoEmptyClipboard", true);
    bUseGUIRegExpEditor = kc->readBoolEntry("UseGUIRegExpEditor", true );
    maxClipItems = kc->readNumEntry("MaxClipItems", 7);
    bIgnoreSelection = kc->readBoolEntry("IgnoreSelection", false);
}

void KlipperWidget::writeConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    kc->writeEntry("PopupAtMousePosition", bPopupAtMouse);
    kc->writeEntry("KeepClipboardContents", bKeepContents);
    kc->writeEntry("ReplayActionInHistory", bReplayActionInHistory);
    kc->writeEntry("NoEmptyClipboard", bNoNullClipboard);
    kc->writeEntry("UseGUIRegExpEditor", bUseGUIRegExpEditor);
    kc->writeEntry("MaxClipItems", maxClipItems);
    kc->writeEntry("IgnoreSelection", bIgnoreSelection);
    kc->writeEntry("Version", klipper_version );

    if ( myURLGrabber )
        myURLGrabber->writeConfiguration( kc );

    kc->sync();
}

// save session on shutdown. Don't simply use the c'tor, as that may not be called.
void KlipperWidget::saveSession()
{
  if ( bKeepContents ) { // save the clipboard eventually
      QStringList dataList;
      if ( !bClipEmpty )
      {
          // don't iterate over the map, but over the popup (due to sorting!)
          long id = 0L;
          for ( uint i = 0; i < m_popup->count(); i++ ) {
              id = m_popup->idAt( i );
              if ( id != -1 ) {
                  QMapIterator<long,QString> it = m_clipDict.find( id );
                  if ( it != m_clipDict.end() )
                      dataList.append( it.data() );
              }
          }
      }

      KConfigGroupSaver groupSaver(m_config, "General");
      m_config->writeEntry("ClipboardData", dataList);
      m_config->sync();
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
    dlg->setStripWhiteSpace( myURLGrabber->stripWhiteSpace() );
    dlg->setReplayActionInHistory( bReplayActionInHistory );
    dlg->setNoNullClipboard( bNoNullClipboard );
    dlg->setUseGUIRegExpEditor( bUseGUIRegExpEditor );
    dlg->setPopupTimeout( myURLGrabber->popupTimeout() );
    dlg->setMaxItems( maxClipItems );
    dlg->setIgnoreSelection( bIgnoreSelection );
    dlg->setNoActionsFor( myURLGrabber->avoidWindows() );
//    dlg->setEnableActions( haveURLGrabber );

    if ( dlg->exec() == QDialog::Accepted ) {
        bKeepContents = dlg->keepContents();
        bPopupAtMouse = dlg->popupAtMousePos();
        bReplayActionInHistory = dlg->replayActionInHistory();
        bNoNullClipboard = dlg->noNullClipboard();
        bIgnoreSelection = dlg->ignoreSelection();
        bUseGUIRegExpEditor = dlg->useGUIRegExpEditor();
        dlg->commitShortcuts();
        // the keys need to be written to kdeglobals, not kickerrc --ellis, 22/9/02
        globalKeys->writeSettings(0, true);
        globalKeys->updateConnections();
        toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

//	haveURLGrabber = dlg->enableActions();

        myURLGrabber->setActionList( dlg->actionList() );
        myURLGrabber->setPopupTimeout( dlg->popupTimeout() );
        myURLGrabber->setStripWhiteSpace( dlg->stripWhiteSpace() );
	myURLGrabber->setAvoidWindows( dlg->noActionsFor() );

	maxClipItems = dlg->maxItems();
	trimClipHistory( maxClipItems );

        // KClipboardSynchronizer configuration
        m_config->setGroup("General");
        m_config->writeEntry("SynchronizeClipboardAndSelection",
                             dlg->synchronize(), true, true );

        writeConfiguration( m_config ); // syncs

        // notify all other apps about new KClipboardSynchronizer configuration
        int message = 0;
        if ( dlg->synchronize() )
            message |= KClipboardSynchronizer::Synchronize;

        KIPC::sendMessageAll( KIPC::ClipboardConfigChanged, message );
    }
    setURLGrabberEnabled( haveURLGrabber );

    delete dlg;
}


void KlipperWidget::slotRepeatAction()
{
    if ( !myURLGrabber ) {
	myURLGrabber = new URLGrabber( m_config );
	connect( myURLGrabber, SIGNAL( sigPopup( QPopupMenu * )),
		 SLOT( showPopupMenu( QPopupMenu * )) );
	connect( myURLGrabber, SIGNAL( sigDisablePopup() ),
			this, SLOT( disableURLGrabber() ) );
    }

    myURLGrabber->invokeAction( m_lastString );
}

void KlipperWidget::setURLGrabberEnabled( bool enable )
{
    bURLGrabber = enable;
    toggleURLGrabAction->setChecked( enable );
    KConfig *kc = m_config;
    kc->setGroup("General");
    kc->writeEntry("URLGrabberEnabled", bURLGrabber);
    kc->sync();

    if ( !bURLGrabber ) {
        delete myURLGrabber;
        myURLGrabber = 0L;
        toggleURLGrabAction->setText(i18n("Enable &Actions"));
    }

    else {
        toggleURLGrabAction->setText(i18n("&Actions Enabled"));
        if ( !myURLGrabber ) {
            myURLGrabber = new URLGrabber( m_config );
            connect( myURLGrabber, SIGNAL( sigPopup( QPopupMenu * )),
                     SLOT( showPopupMenu( QPopupMenu * )) );
	    connect( myURLGrabber, SIGNAL( sigDisablePopup() ),
                     this, SLOT( disableURLGrabber() ) );
        }
    }
}

void KlipperWidget::toggleURLGrabber()
{
    setURLGrabberEnabled( !bURLGrabber );
}

void KlipperWidget::trimClipHistory( int new_size )
{
    while (m_popup->count() - MENU_ITEMS > (unsigned) new_size ) {
        int id = m_popup->idAt(EMPTY);
        if ( id == -1 )
            return;

        m_clipDict.remove(id);
        m_popup->removeItemAt(EMPTY);
    }
}

void KlipperWidget::removeFromHistory( const QString& text )
{
    QMapIterator<long,QString> it = m_clipDict.begin();
    for ( ; it != m_clipDict.end(); ++it ) {
        if ( it.data() == text ) {
            long id = it.key();
            m_popup->removeItem( id );
            m_clipDict.remove( id );
            return; // there can be only one (I hope :)
        }
    }
}

void KlipperWidget::slotClearClipboard()
{
    clip->clear(QClipboard::Selection);
    clip->clear(QClipboard::Clipboard);
    if ( m_selectedItem != -1 )
        m_popup->setItemEnabled(m_selectedItem, false);
}

QString KlipperWidget::clipboardContents( bool *isSelection )
{
    bool selection = true;
    QString contents = clip->text(QClipboard::Selection);

    if ( contents == m_lastSelection )
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

    return contents;
}

void KlipperWidget::applyClipChanges( const QString& clipData )
{
    m_lastString = clipData;

    if ( bURLGrabber && myURLGrabber ) {
        if ( myURLGrabber->checkNewData( clipData ))
            return; // don't add into the history
    }

    if (bClipEmpty) { // remove <clipboard empty> from popupmenu and dict
        if (clipData != QSempty) {
            bClipEmpty = false;
            m_popup->removeItemAt(EMPTY);
            m_clipDict.clear();
        }
    }

    if (m_selectedItem != -1)
        m_popup->setItemChecked(m_selectedItem, false);

    removeFromHistory( clipData );
    trimClipHistory(maxClipItems - 1);

    m_selectedItem = m_popup->insertItem(KStringHandler::cEmSqueeze(clipData.simplifyWhiteSpace(), fontMetrics(), 25).replace( "&", "&&" ), -2, 1); // -2 means unique id, 1 means first location
    m_clipDict.insert(m_selectedItem, clipData);
    if ( bClipEmpty )
        m_popup->setItemEnabled( m_selectedItem, false );
    else
        m_popup->setItemChecked(m_selectedItem, true);
}

void KlipperWidget::setEmptyClipboard()
{
    bClipEmpty = true;
    applyClipChanges( QSempty );
}

void KlipperWidget::slotMoveSelectedToTop()
{
    m_popup->removeItem( m_selectedItem );
    m_clipDict.remove( m_selectedItem );

    m_selectedItem = m_popup->insertItem( KStringHandler::cEmSqueeze(m_lastString.simplifyWhiteSpace(), fontMetrics(), 25).replace( "&", "&&" ), -2, 1 );
    m_popup->setItemChecked( m_selectedItem, true );
    m_clipDict.insert( m_selectedItem, m_lastString );
}

void KlipperWidget::newClipData()
{
    bool selectionMode;
    QString clipContents = clipboardContents( &selectionMode );
//     qDebug("**** newClipData polled: %s", clipContents.latin1());
    checkClipData( clipContents, selectionMode );
}

void KlipperWidget::clipboardSignalArrived( bool selectionMode )
{
    // Don't react on Klipper's own actions. This signal comes either
    // from this process when it sets the clipboard (in which case ignoring
    // it is ok) or when another process sets the clipboard, in which
    // case this process should have got already SelectionClear from that
    // another process and therefore it shouldn't think it owns the clipboard.
    if( selectionMode ? clip->ownsClipboard() : clip->ownsSelection())
        return;

//     qDebug("*** clipboardSignalArrived: %i", selectionMode);

    updateTimestamp();
    QString text = clip->text( selectionMode ? QClipboard::Selection : QClipboard::Clipboard );
    checkClipData( text, selectionMode );
}

void KlipperWidget::checkClipData( const QString& text, bool selectionMode )
{
    if ( ignoreClipboardChanges() ) // internal to klipper, ignoring QSpinBox selections
    {
        // keep our old clipboard, thanks
        setClipboard( selectionMode ? m_lastSelection : m_lastClipboard,
                      selectionMode );
        return;
    }

    bool clipEmpty = (clip->data(selectionMode ? QClipboard::Selection : QClipboard::Clipboard)->format() == 0L);
//     qDebug("checkClipData(%i): %s, empty: %i (lastClip: %s, lastSel: %s)", selectionMode, text.latin1(), clipEmpty, m_lastClipboard.latin1(), m_lastSelection.latin1() );

//     const char *format;
//     int i = 0;
//     while ( (format = clip->data()->format( i++ )) )
//     {
//         qDebug( "    format: %s", format);
//     }

    bool changed = !selectionMode || text != m_lastSelection;

    QString lastClipRef = selectionMode ? m_lastSelection : m_lastClipboard;

    if ( text != lastClipRef ) {
        // keep old clipboard after someone set it to null
        if ( clipEmpty && bNoNullClipboard )
            setClipboard( lastClipRef, selectionMode );
        else
            lastClipRef = text;
    }

    // lastClipRef has the new value now, if any


    // this must be below the "bNoNullClipboard" handling code!
    if ( selectionMode && bIgnoreSelection )
        return;


    // If the string is null bug out
    if (lastClipRef.isEmpty()) {
        if (m_selectedItem != -1) {
            m_popup->setItemChecked(m_selectedItem, false);
            m_selectedItem = -1;
        }

        if ( m_clipDict.isEmpty() ) {
            setEmptyClipboard();
        }
        return;
    }

    // store old contents:
    if ( selectionMode )
        m_lastSelection = lastClipRef;
    else
        m_lastClipboard = lastClipRef;

    if (lastClipRef != m_lastString && changed) {
        applyClipChanges( lastClipRef );
    }
}

void KlipperWidget::setClipboard( const QString& text, bool selectionMode )
{
    setClipboard( text, selectionMode ? Selection : Clipboard );
}

void KlipperWidget::setClipboard( const QString& text, int mode )
{
    bool blocked = clip->signalsBlocked();
    clip->blockSignals( true ); // ### this might break other kicker applets

    KClipboardSynchronizer *klip = KClipboardSynchronizer::self();
    bool selection = klip->isReverseSynchronizing();
    bool synchronize = klip->isSynchronizing();
    klip->setReverseSynchronizing( false );
    klip->setSynchronizing( false );

    if ( mode & Selection ) {
        clip->setText( text, QClipboard::Selection );
    }
    if ( mode & Clipboard ) {
        clip->setText( text, QClipboard::Clipboard );
    }

    klip->setReverseSynchronizing( selection );
    klip->setSynchronizing( synchronize );

    clip->blockSignals( blocked );
}

QStringList KlipperWidget::getClipboardHistoryMenu()
{
    QStringList menu;
    if ( !bClipEmpty )
    {
        // don't iterate over the map, but over the popup (due to sorting!)
        long id = 0L;
        // We skip the title and start at 1
        for ( uint i = 1; i < m_popup->count(); i++ )
        {
            id = m_popup->idAt( i );
            if ( id != -1 )
            {
                QMapIterator<long,QString> it = m_clipDict.find( id );
                if ( it == m_clipDict.end() )
                    break; // End of clipboard entries
                menu << m_popup->text(id);
            }
        }
    }
    return menu;
}

QString KlipperWidget::getClipboardHistoryItem(int i)
{
    if ( !bClipEmpty )
    {
        long id = m_popup->idAt( i+1 ); // Add 1 to skip title
        QMapIterator<long,QString> it = m_clipDict.find( id );
        if ( it != m_clipDict.end() )
            return it.data();
    }
    return QString::null;
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

extern Time qt_x_time;
extern Time qt_x_user_time;

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
{ // Qt3.3.0 and 3.3.1 use qt_x_user_time for clipboard operations
    Time& time = ( strcmp( qVersion(), "3.3.1" ) == 0
                || strcmp( qVersion(), "3.3.0" ) == 0 )
                ? qt_x_user_time : qt_x_time;
    static QWidget* w = 0;
    if ( !w )
        w = new QWidget;
    unsigned char data[ 1 ];
    XChangeProperty( qt_xdisplay(), w->winId(), XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
    next_x_time = CurrentTime;
    XEvent dummy;
    XCheckIfEvent( qt_xdisplay(), &dummy, update_x_time_predicate, NULL );
    if( next_x_time == CurrentTime )
        {
        XSync( qt_xdisplay(), False );
        XCheckIfEvent( qt_xdisplay(), &dummy, update_x_time_predicate, NULL );
        }
    Q_ASSERT( next_x_time != CurrentTime );
    time = next_x_time;
    XEvent ev; // remove the PropertyNotify event from the events queue
    XWindowEvent( qt_xdisplay(), w->winId(), PropertyChangeMask, &ev );
}

static const char * const description =
	I18N_NOOP("KDE Cut & Paste history utility");

void KlipperWidget::createAboutData()
{
  about_data = new KAboutData("klipper", I18N_NOOP("Klipper"),
    klipper_version, description, KAboutData::License_GPL,
		       "(c) 1998, Andrew Stanley-Jones\n"
		       "1998-2002, Carsten Pfeiffer\n"
		       "2001, Patrick Dubroy");

  about_data->addAuthor("Carsten Pfeiffer",
                      I18N_NOOP("Author, Maintainer"),
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
    : KlipperWidget( parent, kapp->config())
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

#include "toplevel.moc"
