/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */

#include <qclipboard.h>
#include <qcursor.h>
#include <qintdict.h>
#include <qmenudata.h>
#include <qpainter.h>
#include <qstrlist.h>

#include <kaction.h>
#include <kapp.h>
#include <kconfig.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kwin.h>
#include <kstringhandler.h>
#include <kiconloader.h>

#include "configdialog.h"
#include "toplevel.h"
#include "urlgrabber.h"


#define QUIT_ITEM    50
#define CONFIG_ITEM  60
#define POPUP_ITEM   70
#define KEEP_ITEM    80
#define URLGRAB_ITEM 90

// the <clipboard empty> item
#define EMPTY 6

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

    toggleURLGrabAction = new KToggleAction( this );
    toggleURLGrabAction->setEnabled( true );

    myURLGrabber = 0L;
    KConfig *kc = kapp->config();
    readConfiguration( kc );
    setURLGrabberEnabled( bURLGrabber );

    QSlast = "";
    pQPMmenu = new KPopupMenu(0x0, "main_menu");
    connect(pQPMmenu, SIGNAL(activated(int)),
            this, SLOT(clickedMenu(int)));

    pQIDclipData = new QIntDict<QString>();
    pQIDclipData->setAutoDelete(TRUE);

    readProperties(kapp->config());
    connect(kapp, SIGNAL(saveYourself()), SLOT(saveProperties()));

    pQTcheck = new QTimer(this, "timer");
    pQTcheck->start(1000, FALSE);
    connect(pQTcheck, SIGNAL(timeout()),
            this, SLOT(newClipData()));
    pQPpic = new QPixmap(mouse);
    resize( pQPpic->size() );

    globalKeys = new KGlobalAccel();
    globalKeys->insertItem(i18n("Show klipper popupmenu"),
                                "show-klipper-popupmenu", "CTRL+ALT+V");
    globalKeys->insertItem(i18n("Repeat last action"),
                                "repeat-last-klipper-action", "CTRL+ALT+R");
    globalKeys->insertItem(i18n("Enable/disable clipboard actions"),
                           "toggle-clipboard-actions", "CTRL+ALT+X" );
    globalKeys->connectItem("show-klipper-popupmenu", this,
                            SLOT(slotPopupMenu()));
    globalKeys->connectItem("repeat-last-klipper-action", this,
                            SLOT(slotRepeatAction()));
    globalKeys->connectItem("toggle-clipboard-actions", this,
                            SLOT( toggleURLGrabber()));
    globalKeys->readSettings();
    toggleURLGrabAction->setAccel( globalKeys->currentKey( "toggle-clipboard-actions" ));


    connect( toggleURLGrabAction, SIGNAL( toggled( bool ) ), this,
             SLOT( setURLGrabberEnabled( bool )));
    setBackgroundMode(X11ParentRelative);
}

TopLevel::~TopLevel()
{
    delete globalKeys;
    delete pQTcheck;
    delete pQPMmenu;
    delete pQIDclipData;
    delete pQPpic;
    delete myURLGrabber;
}

void TopLevel::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() == LeftButton || e->button() == RightButton )
        showPopupMenu( pQPMmenu );
}

void TopLevel::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  int x = (width() - pQPpic->width()) / 2;
  int y = (height() - pQPpic->height()) / 2;
  if ( x < 0 ) x = 0;
  if ( y < 0 ) y = 0;
  p.drawPixmap(x , y, *pQPpic);
  p.end();
}

void TopLevel::newClipData()
{
    QString clipData = clip->text().stripWhiteSpace();
    // If the string is null bug out
    if(clipData.isEmpty())
        return;

    if(clipData != QSlast) {
        QSlast = clipData.copy();

        QString *data = new QString(clipData);

        if ( myURLGrabber ) {
            if ( myURLGrabber->checkNewData( clipData ))
                return; // don't add into the history
        }


        if (bClipEmpty) { // remove <clipboard empty> from popupmenu and dict
            if (*data != QSempty) {
                bClipEmpty = false;
                pQPMmenu->removeItemAt(EMPTY);
                pQIDclipData->clear();
            }
        }

        while(pQPMmenu->count() > 12){
            int id = pQPMmenu->idAt(EMPTY);
            pQIDclipData->remove(id);
            pQPMmenu->removeItemAt(EMPTY);

        }
        if(clipData.length() > 50){
            clipData.truncate(47);
            clipData.append("...");
        }
        long int id = pQPMmenu->insertItem(clipData.simplifyWhiteSpace(), -2, -1); // -2 means unique id, -1 means at end
        pQIDclipData->insert(id, data);
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
    case QUIT_ITEM:
        saveProperties();
        kapp->quit();
        break;
    default:
        pQTcheck->stop();
        QString *data = pQIDclipData->find(id);
        if(data != 0x0 && *data != QSempty){
            clip->setText(*data);
            QSlast = data->copy();
        }

        pQTcheck->start(1000);
    }
}


void TopLevel::showPopupMenu( QPopupMenu *menu )
{
    ASSERT( menu != 0L );

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
  pQPMmenu->clear();
  pQPMmenu->insertTitle(kapp->miniIcon(), i18n("Klipper - Clipboard Tool"));
  pQPMmenu->insertItem(SmallIcon("exit"), i18n("&Quit"), QUIT_ITEM );
  pQPMmenu->insertSeparator();
  pQPMmenu->insertItem(SmallIcon("configure"), i18n("&Preferences..."), CONFIG_ITEM);
  toggleURLGrabAction->plug( pQPMmenu );
  pQPMmenu->insertSeparator();
  long int id;

  if (bKeepContents) { // load old clipboard if configured
      KConfigGroupSaver groupSaver(kc, "General");
      dataList = kc->readListEntry("ClipboardData");

      for (QStringList::ConstIterator it = dataList.begin();
           it != dataList.end(); ++it)
      {
          id = pQPMmenu->insertItem( KStringHandler::csqueeze(*it, 40), -2, -1 );
          pQIDclipData->insert( id, new QString( *it ));
      }
  }

  QSempty = i18n("<empty clipboard>");
  bClipEmpty = ((QString)clip->text()).simplifyWhiteSpace().isEmpty() && dataList.isEmpty();

  if(bClipEmpty)
    clip->setText(QSempty);
  newClipData();
}


void TopLevel::readConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    bPopupAtMouse = kc->readBoolEntry("PopupAtMousePosition", false);
    bKeepContents = kc->readBoolEntry("KeepClipboardContents", true);
    bURLGrabber = kc->readBoolEntry("URLGrabberEnabled", false);
}

void TopLevel::writeConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    kc->writeEntry("PopupAtMousePosition", bPopupAtMouse);
    kc->writeEntry("KeepClipboardContents", bKeepContents);

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
      QIntDictIterator<QString> it( *pQIDclipData );

      while ( (data = it.current()) ) {
          dataList.prepend( *it );
          ++it;
      }

      kc->writeEntry("ClipboardData", dataList);
      kc->sync();
  }
}


void TopLevel::slotConfigure()
{
    if ( !myURLGrabber ) { // temporary, for the config-dialog
        setURLGrabberEnabled( true );
        readConfiguration( kapp->config() );
    }

    KKeyEntryMap map = globalKeys->keyDict();
    ConfigDialog *dlg = new ConfigDialog( myURLGrabber->actionList(),
                                          &map );
    dlg->setKeepContents( bKeepContents );
    dlg->setPopupAtMousePos( bPopupAtMouse );
    dlg->setPopupTimeout( myURLGrabber->popupTimeout() );

    if ( dlg->exec() == QDialog::Accepted ) {
        bKeepContents = dlg->keepContents();
        bPopupAtMouse = dlg->popupAtMousePos();
        globalKeys->setKeyDict( map );
        globalKeys->writeSettings();
        toggleURLGrabAction->setAccel( globalKeys->currentKey( "toggle-clipboard-actions" ));

        myURLGrabber->setActionList( dlg->actionList() );
	myURLGrabber->setPopupTimeout( dlg->popupTimeout() );
        writeConfiguration( kapp->config() );
        setURLGrabberEnabled( bURLGrabber );
    }

    else // eventually delete the temporary urlgrabber, when cancel was pressed
        setURLGrabberEnabled( bURLGrabber );

    delete dlg;
}


void TopLevel::slotRepeatAction()
{
    if ( myURLGrabber && bURLGrabber )
        myURLGrabber->repeatLastAction();
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
        toggleURLGrabAction->setText(i18n("Enable &actions"));
    }

    else {
        toggleURLGrabAction->setText(i18n("&Actions enabled"));
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
#include "toplevel.moc"
