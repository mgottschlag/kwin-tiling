/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */

#include <qclipboard.h>
#include <qcursor.h>
#include <qfile.h>
#include <qintdict.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <kaboutdata.h>
#include <kaccelaction.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <kwin.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "configdialog.h"
#include "toplevel.h"
#include "urlgrabber.h"


#define QUIT_ITEM    50
#define CONFIG_ITEM  60
#define EMPTY_ITEM   80

#define MENU_ITEMS   (7 + (KGlobalSettings::insertTearOffHandle() ? 1 : 0 ))
// the <clipboard empty> item
#define EMPTY (m_popup->count() - MENU_ITEMS)


/* XPM */
static const char*mouse[]={
"20 20 8 1",
"d c #ffa858",
"e c #c05800",
"# c #000000",
"c c #ff8000",
". c None",
"b c #a0a0a4",
"a c #dcdcdc",
"f c #ffffff",
".....###########....",
"..####aaaaaaab####..",
".#cccc#a....b#cccc#.",
".#cd####abbb####de#.",
".#cd#fff####ff.#de#.",
".#cd#fffffffff.#de#.",
".#cd#f.######f.#de#.",
".#cd#fffffffff.#de#.",
".#cd#f.#####ff.#de#.",
".#cd#fffffffff.#de#.",
".#cd#ff#ff#fff.#de#.",
".#cd#ff#f#ffff.#de#.",
".#cd#ff##fffff.#de#.",
".#cd#ff#f#ffff.#de#.",
".#cd#ff#ff#fff.#de#.",
".#cd#fffffffff.#de#.",
".#cd#..........#de#.",
".#cd############de#.",
".#ccccccccccccccde#.",
"..################.."};

TopLevel::TopLevel()
  : KMainWindow(0)
{
    clip = kapp->clipboard();
    m_selectedItem = -1;

    QSempty = i18n("<empty clipboard>");

    toggleURLGrabAction = new KToggleAction( this );
    toggleURLGrabAction->setEnabled( true );

    myURLGrabber = 0L;
    KConfig *kc = kapp->config();
    readConfiguration( kc );
    setURLGrabberEnabled( bURLGrabber );

    m_lastString = "";
    m_popup = new KPopupMenu(0L, "main_menu");
    connect(m_popup, SIGNAL(activated(int)),
            this, SLOT(clickedMenu(int)));

    m_clipDict = new QIntDict<QString>();
    m_clipDict->setAutoDelete(TRUE);

    readProperties(kapp->config());
    connect(kapp, SIGNAL(saveYourself()), SLOT(saveProperties()));

    m_checkTimer = new QTimer(this, "timer");
    m_checkTimer->start(1000, FALSE);
    connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(newClipData()));
    m_pixmap = new QPixmap(mouse);
    resize( m_pixmap->size() );

    globalKeys = new KGlobalAccel(this);
    KGlobalAccel* keys = globalKeys;
#include "klipperbindings.cpp"
    globalKeys->readSettings();
    globalKeys->updateConnections();
    KAccelAction* pAction = globalKeys->actions().actionPtr("Enable/Disable Clipboard Actions");
    uint keyCombQt = pAction->shortcut().keyPrimaryQt();
    toggleURLGrabAction->setAccel(keyCombQt);

    connect( toggleURLGrabAction, SIGNAL( toggled( bool )), this,
	     SLOT( setURLGrabberEnabled( bool )));
    setBackgroundMode(X11ParentRelative);

    QToolTip::add( this, i18n("Klipper - Clipboard Tool") );
}

TopLevel::~TopLevel()
{
    delete m_checkTimer;
    delete m_popup;
    delete m_clipDict;
    delete m_pixmap;
    delete myURLGrabber;
}

void TopLevel::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() == LeftButton || e->button() == RightButton )
        showPopupMenu( m_popup );
}

void TopLevel::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  int x = (width() - m_pixmap->width()) / 2;
  int y = (height() - m_pixmap->height()) / 2;
  if ( x < 0 ) x = 0;
  if ( y < 0 ) y = 0;
  p.drawPixmap(x , y, *m_pixmap);
  p.end();
}

void TopLevel::newClipData()
{
    QString clipData;

    clip->setSelectionMode( false );
    QString clipContents = clip->text().stripWhiteSpace();
    clip->setSelectionMode( true );
    QString selectContents = clip->text().stripWhiteSpace();

    if ( clipContents != m_lastClipboard ) {
        // keep old clipboard after someone set it to null
        if ( clipContents.isNull() && bNoNullClipboard ) {
            clipContents = m_lastClipboard;
            clip->setSelectionMode( false );
            clip->setText( clipContents );
        }
        
        clipData        = clipContents;
        m_lastClipboard = clipContents;
        // sync clipboard and selection?
        if( bSynchronize && clipContents != selectContents ) {
            m_lastSelection = clipContents;
            clip->setSelectionMode( true );
            clip->setText(clipContents);
        }

    }
    else {
        if ( selectContents != m_lastSelection ) {
            // keep old selection after someone set it to null
            if ( selectContents.isNull() && bNoNullClipboard ) {
                selectContents = m_lastSelection;
                clip->setSelectionMode( true );
                clip->setText( selectContents );
            }
            
            clipData        = selectContents;
            m_lastSelection = selectContents;
            // sync clipboard and selection?
            if( bSynchronize && selectContents != clipContents ) {
                m_lastClipboard = selectContents;
                clip->setSelectionMode( false );
                clip->setText(selectContents);
            }
        }
        else
            clipData = m_lastString;
    }

    // If the string is null bug out
    if (clipData.isEmpty()) {
	if (m_selectedItem != -1) {
            m_popup->setItemChecked(m_selectedItem, false);
	    m_selectedItem = -1;
	}

        if ( m_clipDict->isEmpty() ) {
            setEmptyClipboard();
        }
        return;
    }

    if (clipData != m_lastString) {
        slotClipboardChanged( clipData );
    }
}

void TopLevel::clickedMenu(int id)
{
    switch ( id ) {
    case -1:
        break;
    case CONFIG_ITEM:
        slotConfigure();
        break;
    case QUIT_ITEM: {
        saveProperties();
        int autoStart = KMessageBox::questionYesNoCancel( 0L, i18n("Should Klipper start automatically\nwhen you login?"), i18n("Automatically Start Klipper?") );

        QString file = locateLocal( "data", "../autostart/klipper.desktop" );
        if ( autoStart == KMessageBox::Yes )
            QFile::remove( file );
        else if ( autoStart == KMessageBox::No) {
            KSimpleConfig config( file );
            config.setDesktopGroup();
            config.writeEntry( "Hidden", true );
            config.sync();
        }else  // cancel chosen don't quit
	    break;
        kapp->quit();
        break;
        }
//    case URLGRAB_ITEM: // handled with an extra slot
//	break;
    case EMPTY_ITEM:
	if ( !bClipEmpty )
	{
	    m_checkTimer->stop();

	    trimClipHistory(0);
            slotClearClipboard();
            setEmptyClipboard();

	    m_checkTimer->start(1000);
	}
	break;
    default:
	if ( id == URLGrabItem )
	{
	    break; // handled by its own slot
	}
	else if ( !bClipEmpty )
	{
	    m_checkTimer->stop();
	    //CT mark up the currently put into clipboard -
            // so that user can see later
            if ( m_selectedItem != -1 )
                m_popup->setItemChecked(m_selectedItem, false);

	    m_selectedItem = id;
	    m_popup->setItemChecked(m_selectedItem, true);
	    QString *data = m_clipDict->find(id);
	    if (data != 0x0 && *data != QSempty)
            {
                clip->setSelectionMode( true );
		clip->setText( *data );
                clip->setSelectionMode( false );
                clip->setText( *data );

		if (bURLGrabber && bReplayActionInHistory)
		    myURLGrabber->checkNewData(*data);

                m_lastString = *data;

                // We want to move the just selected item to the top of the popup
                // menu. But when we do this right here, we get a crash a little
                // bit later. So instead, we fire a timer to perform the moving.
                QTimer::singleShot( 0, this, SLOT( slotMoveSelectedToTop() ));
            }

	    m_checkTimer->start(1000);
	}
    }
}


void TopLevel::showPopupMenu( QPopupMenu *menu )
{
    Q_ASSERT( menu != 0L );

    menu->move(-1000,-1000);
    menu->show();
    menu->hide();

    if (bPopupAtMouse) {
        QPoint g = QCursor::pos();
        if ( menu->height() < g.y() )
            menu->popup(QPoint( g.x(), g.y() - menu->height()));
        else
            menu->popup(QPoint(g.x(), g.y()));
    }

    else {
        KWin::Info i = KWin::info( winId() );
        QRect g = i.geometry;

        if ( g.x() > QApplication::desktop()->width()/2 &&
             g.y() + menu->height() > QApplication::desktop()->height() )
            menu->popup(QPoint( g.x(), g.y() - menu->height()));
        else
            menu->popup(QPoint( g.x() + width(), g.y() + height()));

        //      menu->exec(mapToGlobal(QPoint( width()/2, height()/2 )));
    }
}


void TopLevel::readProperties(KConfig *kc)
{
  QStringList dataList;

  m_popup->clear();
  m_popup->insertTitle(kapp->miniIcon(), i18n("Klipper - Clipboard Tool"));

  if (bKeepContents) { // load old clipboard if configured
      KConfigGroupSaver groupSaver(kc, "General");
      dataList = kc->readListEntry("ClipboardData");

      for (QStringList::ConstIterator it = dataList.begin();
           it != dataList.end(); ++it)
      {
          long id = m_popup->insertItem( KStringHandler::csqueeze(*it, 45), -2, -1);
          m_clipDict->insert( id, new QString( *it ));
      }
  }

  bClipEmpty = clipboardContents().simplifyWhiteSpace().isEmpty() && dataList.isEmpty();

  m_popup->insertSeparator();
  toggleURLGrabAction->plug( m_popup, -1 );
  URLGrabItem = m_popup->idAt( m_popup->count() - 1 );

  m_popup->insertItem( SmallIcon("fileclose"),
			i18n("&Clear Clipboard History"), EMPTY_ITEM );
  m_popup->insertItem(SmallIcon("configure"), i18n("&Preferences..."),
		       CONFIG_ITEM);
  m_popup->insertSeparator();
  m_popup->insertItem(SmallIcon("exit"), i18n("&Quit"), QUIT_ITEM );
  m_popup->insertTearOffHandle();

  if (bClipEmpty)
      setEmptyClipboard();
}


void TopLevel::readConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    bPopupAtMouse = kc->readBoolEntry("PopupAtMousePosition", false);
    bKeepContents = kc->readBoolEntry("KeepClipboardContents", true);
    bURLGrabber = kc->readBoolEntry("URLGrabberEnabled", true);
    bReplayActionInHistory = kc->readBoolEntry("ReplayActionInHistory", false);
    bSynchronize = kc->readBoolEntry("SynchronizeClipboards", false);
    bNoNullClipboard = kc->readBoolEntry("NoEmptyClipboard", true);
    bUseGUIRegExpEditor = kc->readBoolEntry("UseGUIRegExpEditor", true );
    maxClipItems = kc->readNumEntry("MaxClipItems", 7);
}

void TopLevel::writeConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    kc->writeEntry("PopupAtMousePosition", bPopupAtMouse);
    kc->writeEntry("KeepClipboardContents", bKeepContents);
    kc->writeEntry("ReplayActionInHistory", bReplayActionInHistory);
    kc->writeEntry("SynchronizeClipboards", bSynchronize);
    kc->writeEntry("NoEmptyClipboard", bNoNullClipboard);
    kc->writeEntry("UseGUIRegExpEditor", bUseGUIRegExpEditor);
    kc->writeEntry("MaxClipItems", maxClipItems);
    kc->writeEntry("Version", kapp->aboutData()->version());

    if ( myURLGrabber )
        myURLGrabber->writeConfiguration( kc );

    kc->sync();
}


// session management
void TopLevel::saveProperties()
{
  if (bKeepContents) { // save the clipboard eventually
      QString  *data;
      QStringList dataList;
      KConfig  *kc = kapp->config();
      KConfigGroupSaver groupSaver(kc, "General");
      QIntDictIterator<QString> it( *m_clipDict );
      if ( !bClipEmpty )
      {
	  while ( (data = it.current()) ) {
	      dataList.prepend( *it );
	      ++it;
	  }
      }
      kc->writeEntry("ClipboardData", dataList);
      kc->sync();
  }
}


void TopLevel::slotConfigure()
{
    bool haveURLGrabber = bURLGrabber;
    if ( !myURLGrabber ) { // temporary, for the config-dialog
        setURLGrabberEnabled( true );
        readConfiguration( kapp->config() );
    }

    KAccelActions map = globalKeys->actions();
    ConfigDialog *dlg = new ConfigDialog( myURLGrabber->actionList(),
                                          map );
    dlg->setKeepContents( bKeepContents );
    dlg->setPopupAtMousePos( bPopupAtMouse );
    dlg->setReplayActionInHistory( bReplayActionInHistory );
    dlg->setSynchronize( bSynchronize );
    dlg->setNoNullClipboard( bNoNullClipboard );
    dlg->setUseGUIRegExpEditor( bUseGUIRegExpEditor );
    dlg->setPopupTimeout( myURLGrabber->popupTimeout() );
    dlg->setMaxItems( maxClipItems );
    dlg->setNoActionsFor( myURLGrabber->avoidWindows() );
//    dlg->setEnableActions( haveURLGrabber );

    if ( dlg->exec() == QDialog::Accepted ) {
        bKeepContents = dlg->keepContents();
        bPopupAtMouse = dlg->popupAtMousePos();
        bReplayActionInHistory = dlg->replayActionInHistory();
        bSynchronize = dlg->synchronize();
        bNoNullClipboard = dlg->noNullClipboard();
        bUseGUIRegExpEditor = dlg->useGUIRegExpEditor();
        globalKeys->actions().updateShortcuts( map );
        globalKeys->writeSettings();
        globalKeys->updateConnections();
        KAccelAction* pAction = globalKeys->actions().actionPtr("Enable/Disable Clipboard Actions");
        uint keyCombQt = pAction->shortcut().keyPrimaryQt();
        toggleURLGrabAction->setAccel(keyCombQt);

//	haveURLGrabber = dlg->enableActions();

        myURLGrabber->setActionList( dlg->actionList() );
        myURLGrabber->setPopupTimeout( dlg->popupTimeout() );
	myURLGrabber->setAvoidWindows( dlg->noActionsFor() );

	maxClipItems = dlg->maxItems();
	trimClipHistory( maxClipItems );
	
        writeConfiguration( kapp->config() );
    }
    setURLGrabberEnabled( haveURLGrabber );

    delete dlg;
}


void TopLevel::slotRepeatAction()
{
    if ( !myURLGrabber ) {
	myURLGrabber = new URLGrabber();
	connect( myURLGrabber, SIGNAL( sigPopup( QPopupMenu * )),
		 SLOT( showPopupMenu( QPopupMenu * )) );
    }

    myURLGrabber->invokeAction( m_lastString );
}

void TopLevel::setURLGrabberEnabled( bool enable )
{
    bURLGrabber = enable;
    toggleURLGrabAction->setChecked( enable );
    KConfig *kc = kapp->config();
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
            myURLGrabber = new URLGrabber();
            connect( myURLGrabber, SIGNAL( sigPopup( QPopupMenu * )),
                     SLOT( showPopupMenu( QPopupMenu * )) );
        }
    }
}

void TopLevel::toggleURLGrabber()
{
    setURLGrabberEnabled( !bURLGrabber );
}

void TopLevel::trimClipHistory( int new_size )
{
    while (m_popup->count() - MENU_ITEMS > (unsigned) new_size ) {
        int id = m_popup->idAt(EMPTY);
        if ( id == -1 )
            return;

        m_clipDict->remove(id);
        m_popup->removeItemAt(EMPTY);
    }
}

void TopLevel::removeFromHistory( const QString& text )
{
    QIntDictIterator<QString> it( *m_clipDict );
    while ( it.current() ) {
        if ( *(it.current()) == text ) {
            long id = it.currentKey();
            m_popup->removeItem( id );
            m_clipDict->remove( id );
        }

        ++it;
    }
}

void TopLevel::slotClearClipboard()
{
    clip->setSelectionMode( true );
    clip->clear();
    clip->setSelectionMode( false );
    clip->clear();
    if ( m_selectedItem != -1 )
        m_popup->setItemEnabled(m_selectedItem, false);
}

QString TopLevel::clipboardContents()
{
    clip->setSelectionMode( true );
    QString clipContents = clip->text().stripWhiteSpace();
    if ( clipContents.isEmpty() ) {
        clip->setSelectionMode( false );
        clipContents = clip->text().stripWhiteSpace();
    }

    return clipContents;
}

void TopLevel::slotClipboardChanged( const QString& clipData )
{
    m_lastString = clipData;

    QString *data = new QString(clipData);

    if ( bURLGrabber && myURLGrabber ) {
        if ( myURLGrabber->checkNewData( clipData ))
            return; // don't add into the history
    }

    if (bClipEmpty) { // remove <clipboard empty> from popupmenu and dict
        if (*data != QSempty) {
            bClipEmpty = false;
            m_popup->removeItemAt(EMPTY);
            m_clipDict->clear();
        }
    }

    if (m_selectedItem != -1)
        m_popup->setItemChecked(m_selectedItem, false);

    removeFromHistory( clipData );
    trimClipHistory(maxClipItems - 1);

    m_selectedItem = m_popup->insertItem(KStringHandler::csqueeze(clipData.simplifyWhiteSpace(), 45), -2, 1); // -2 means unique id, 1 means first location
    m_clipDict->insert(m_selectedItem, data);
    if ( bClipEmpty )
        m_popup->setItemEnabled( m_selectedItem, false );
    else
        m_popup->setItemChecked(m_selectedItem, true);
}

void TopLevel::setEmptyClipboard()
{
    bClipEmpty = true;
    slotClipboardChanged( QSempty );
}

void TopLevel::slotMoveSelectedToTop()
{
    m_popup->removeItem( m_selectedItem );
    m_clipDict->remove( m_selectedItem );

    m_selectedItem = m_popup->insertItem( KStringHandler::csqueeze(m_lastString.simplifyWhiteSpace(), 45), -2, 1 );
    m_popup->setItemChecked( m_selectedItem, true );
    m_clipDict->insert( m_selectedItem, new QString( m_lastString ) );
}

#include "toplevel.moc"
