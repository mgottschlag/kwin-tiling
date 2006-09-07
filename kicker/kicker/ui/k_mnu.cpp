/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dmctl.h>
#include <QtDBus/QtDBus>

#include <QDesktopWidget>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QHash>
#include <QMenu>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionFrame>
#include <kapplication.h>
#include <kaboutkde.h>
#include <kactioncollection.h>
#include <kbookmarkmenu.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <kauthorized.h>
#include <kworkspace.h>

#include "utils.h"

#include "container_base.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "konqbookmarkmanager.h"
#include "menuinfo.h"
#include "quickbrowser_mnu.h"
#include "recentapps.h"

#include "k_mnu.h"
#include "kdesktop_interface.h"
#include "kdesktop_screensaver_interface.h"
#include "k_mnu.moc"

PanelKMenu::PanelKMenu()
  : PanelServiceMenu(QString(), QString(), 0, "KMenu")
  , bookmarkMenu(0)
{
    // set the first client id to some arbitrarily large value.
    client_id = 10000;
    // Don't automatically clear the main menu.
    disableAutoClear();
    actionCollection = new KActionCollection(this);
    setWindowTitle(i18n("K Menu"));
    connect(Kicker::self(), SIGNAL(configurationChanged()),
            this, SLOT(configChanged()));

/* FIXME: we need a proper way to file recent app/doc usage
    DCOPClient *dcopClient = KApplication::dcopClient();
    dcopClient->connectDCOPSignal(0, "appLauncher",
        "serviceStartedByStorageId(QString,QString)",
        dcopObjId,
        "slotServiceStartedByStorageId(QString,QString)",
        false);
    */
}

PanelKMenu::~PanelKMenu()
{
    clearSubmenus();
    delete bookmarkMenu;
}
#if 0
void PanelKMenu::slotServiceStartedByStorageId(QString starter,
                                               QString storageId)
{
    if (starter != "kmenu")
    {
        kDebug() << "KMenu - updating recently used applications: " <<
            storageId << endl;
        KService::Ptr service = KService::serviceByStorageId(storageId);
        RecentlyLaunchedApps::self().updateRecentlyUsedApps(service);
    }
}
#endif

bool PanelKMenu::loadSidePixmap()
{
    if (!KickerSettings::useSidePixmap())
    {
        return false;
    }

    QString sideName = KickerSettings::sidePixmapName();
    QString sideTileName = KickerSettings::sideTileName();

    QImage image;
    image.load(KStandardDirs::locate("data", "kicker/pics/" + sideName));

    if (image.isNull())
    {
        kDebug(1210) << "Can't find a side pixmap" << endl;
        return false;
    }

    Plasma::colorize(image);
    sidePixmap = QPixmap::fromImage(image);

    image.load(KStandardDirs::locate("data", "kicker/pics/" + sideTileName));

    if (image.isNull())
    {
        kDebug(1210) << "Can't find a side tile pixmap" << endl;
        return false;
    }

    Plasma::colorize(image);
    sideTilePixmap = QPixmap::fromImage(image);

    if (sidePixmap.width() != sideTilePixmap.width())
    {
        kDebug(1210) << "Pixmaps have to be the same size" << endl;
        return false;
    }

    // pretile the pixmap to a height of at least 100 pixels
    if (sideTilePixmap.height() < 100)
    {
        int tiles = (int)(100 / sideTilePixmap.height()) + 1;
        QPixmap preTiledPixmap(sideTilePixmap.width(), sideTilePixmap.height() * tiles);
        QPainter p(&preTiledPixmap);
        p.drawTiledPixmap(preTiledPixmap.rect(), sideTilePixmap);
        sideTilePixmap = preTiledPixmap;
    }

    return true;
}

void PanelKMenu::paletteChanged()
{
    if (!loadSidePixmap())
    {
        sidePixmap = sideTilePixmap = QPixmap();
        setMinimumSize( sizeHint() );
    }
}

void PanelKMenu::initialize()
{
//    kDebug(1210) << "PanelKMenu::initialize()" << endl;
    updateRecent();

    if (initialized())
    {
        return;
    }

    if (loadSidePixmap())
    {
        // in case we've been through here before, let's disconnect
        disconnect(kapp, SIGNAL(kdisplayPaletteChanged()),
                   this, SLOT(paletteChanged()));
        connect(kapp, SIGNAL(kdisplayPaletteChanged()),
                this, SLOT(paletteChanged()));
    }
    else
    {
        sidePixmap = sideTilePixmap = QPixmap();
    }

    // add services
    PanelServiceMenu::initialize();

    /*
     FIXME: no more insertTitle! now what?
    if (KickerSettings::showMenuTitles())
    {
        int id;
        id = insertTitle(i18n("All Applications"), -1, 0);
        setItemEnabled( id, false );
        id = insertTitle(i18n("Actions"), -1 , -1);
        setItemEnabled( id, false );
    }
    */

    // create recent menu section
    createRecentMenuItems();

    bool need_separator = false;

    // insert bookmarks
    if (KickerSettings::useBookmarks() && KAuthorized::authorizeKAction("bookmarks"))
    {
        // Need to create a new popup each time, it's deleted by subMenus.clear()
        KMenu * bookmarkParent = new KMenu(this);
        bookmarkParent->setObjectName("bookmarks" );
        delete bookmarkMenu; // can't reuse old one, the popup has been deleted
        bookmarkMenu = new KBookmarkMenu( KonqBookmarkManager::self(), 0, bookmarkParent, actionCollection );

        insertItem(Plasma::menuIconSet("bookmark"),
                   i18n("Bookmarks"), bookmarkParent);

        subMenus.append(bookmarkParent);
        need_separator = true;
    }

    // insert quickbrowser
    if (KickerSettings::useBrowser())
    {
        PanelQuickBrowser *browserMnu = new PanelQuickBrowser(this);
        browserMnu->initialize();

        insertItem(Plasma::menuIconSet("kdisknav"),
                   i18n("Quick Browser"),
                   Plasma::reduceMenu(browserMnu));
        subMenus.append(browserMnu);
        need_separator = true;
    }

    // insert dynamic menus
    QStringList menu_ext = KickerSettings::menuExtensions();
    if (!menu_ext.isEmpty())
    {
        for (QStringList::ConstIterator it=menu_ext.begin(); it!=menu_ext.end(); ++it)
        {
            MenuInfo info(*it);
            if (!info.isValid())
               continue;

            KPanelMenu *menu = info.load();
            if (menu)
            {
                insertItem(Plasma::menuIconSet(info.icon()), info.name(), menu);
                dynamicSubMenus.append(menu);
                need_separator = true;
            }
        }
    }

    if (need_separator)
        addSeparator();

    // run command
    if (KAuthorized::authorizeKAction("run_command"))
    {
        insertItem(Plasma::menuIconSet("run"),
                   i18n("Run Command..."),
                   this,
                   SLOT( slotRunCommand()));
        addSeparator();
    }

    if (DM().isSwitchable() && KAuthorized::authorizeKAction("switch_user"))
    {
        sessionsMenu = new QMenu( this );
        insertItem(Plasma::menuIconSet("switchuser"),
                   i18n("Switch User"), sessionsMenu);
        connect( sessionsMenu, SIGNAL(aboutToShow()), SLOT(slotPopulateSessions()) );
        connect( sessionsMenu, SIGNAL(activated(int)), SLOT(slotSessionActivated(int)) );
    }

    /*
      If  the user configured ksmserver to
    */
    KConfig ksmserver("ksmserverrc", false, false);
    ksmserver.setGroup("General");
    if (ksmserver.readEntry( "loginMode" ) == "restoreSavedSession")
    {
        insertItem(Plasma::menuIconSet("filesave"),
                   i18n("Save Session"), this, SLOT(slotSaveSession()));
    }

    if (KAuthorized::authorizeKAction("lock_screen"))
    {
        insertItem(Plasma::menuIconSet("lock"),
                   i18n("Lock Session"), this, SLOT(slotLock()));
    }

    if (KAuthorized::authorizeKAction("logout"))
    {
        insertItem(Plasma::menuIconSet("exit"),
                   i18n("Log Out..."), this, SLOT(slotLogout()));
    }

#if 0
    // WABA: tear off handles don't work together with dynamically updated
    // menus. We can't update the menu while torn off, and we don't know
    // when it is torn off.
    if (KGlobalSettings::insertTearOffHandle())
      insertTearOffHandle();
#endif

    setInitialized(true);
}

extern int kicker_screen_number;

void PanelKMenu::slotLock()
{
    QString interface( "org.kde.kdesktop" );
    if ( kicker_screen_number )
        interface.sprintf("org.kde.kdesktop-screen-%d", kicker_screen_number);

    org::kde::kdesktop::ScreenSaver screenSaverInterface( interface, "/ScreenSaver", QDBusConnection::sessionBus() );
    if ( screenSaverInterface.isValid() )
        screenSaverInterface.lock();
}

void PanelKMenu::slotLogout()
{
    KWorkSpace::requestShutDown();
}

void PanelKMenu::slotPopulateSessions()
{
    int p = 0;
    DM dm;

    sessionsMenu->clear();
    if (KAuthorized::authorizeKAction("start_new_session") && (p = dm.numReserve()) >= 0)
    {
        if (KAuthorized::authorizeKAction("lock_screen"))
	  sessionsMenu->insertItem(/*SmallIconSet("lockfork"),*/ i18n("Lock Current && Start New Session"), 100 );
        sessionsMenu->insertItem(SmallIconSet("fork"), i18n("Start New Session"), 101 );
        if (!p) {
            sessionsMenu->setItemEnabled( 100, false );
            sessionsMenu->setItemEnabled( 101, false );
        }
        sessionsMenu->addSeparator();
    }
    SessList sess;
    if (dm.localSessions( sess ))
        for (SessList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
            int id = sessionsMenu->insertItem( DM::sess2Str( *it ), (*it).vt );
            if (!(*it).vt)
                sessionsMenu->setItemEnabled( id, false );
            if ((*it).self)
                sessionsMenu->setItemChecked( id, true );
        }
}

void PanelKMenu::slotSessionActivated( int ent )
{
    if (ent == 100)
        doNewSession( true );
    else if (ent == 101)
        doNewSession( false );
    else if (!sessionsMenu->isItemChecked( ent ))
        DM().lockSwitchVT( ent );
}

void PanelKMenu::doNewSession( bool lock )
{
    int result = KMessageBox::warningContinueCancel(
        kapp->desktop()->screen(kapp->desktop()->screenNumber(this)),
        i18n("<p>You have chosen to open another desktop session.<br>"
               "The current session will be hidden "
               "and a new login screen will be displayed.<br>"
               "An F-key is assigned to each session; "
               "F%1 is usually assigned to the first session, "
               "F%2 to the second session and so on. "
               "You can switch between sessions by pressing "
               "Ctrl, Alt and the appropriate F-key at the same time. "
               "Additionally, the KDE Panel and Desktop menus have "
               "actions for switching between sessions.</p>",
                            7, 8),
        i18n("Warning - New Session"),
        KGuiItem(i18n("&Start New Session"), "fork"),
        ":confirmNewSession",
        KMessageBox::PlainCaption | KMessageBox::Notify);

    if (result==KMessageBox::Cancel)
        return;

    if (lock)
        slotLock();

    DM().startReserve();
}

void PanelKMenu::slotSaveSession()
{
    QDBusInterface ksmserver("org.kde.ksmserver", "/ksmserver", "org.kde.KSMServerInterface");
    ksmserver.call("saveCurrentSession");
}

void PanelKMenu::slotRunCommand()
{
    QString interface( "org.kde.kdesktop" );
    if ( kicker_screen_number )
        interface.sprintf("org.kde.kdesktop-screen-%d", kicker_screen_number);

    org::kde::kdesktop::Desktop desktopInterface( interface, "/Desktop", QDBusConnection::sessionBus() );
    desktopInterface.popupExecuteCommand( QString() );
}

void PanelKMenu::slotEditUserContact()
{
}

void PanelKMenu::setMinimumSize(const QSize & s)
{
    KPanelMenu::setMinimumSize(s.width() + sidePixmap.width(), s.height());
}

void PanelKMenu::setMaximumSize(const QSize & s)
{
    KPanelMenu::setMaximumSize(s.width() + sidePixmap.width(), s.height());
}

void PanelKMenu::setMinimumSize(int w, int h)
{
    KPanelMenu::setMinimumSize(w + sidePixmap.width(), h);
}

void PanelKMenu::setMaximumSize(int w, int h)
{
  KPanelMenu::setMaximumSize(w + sidePixmap.width(), h);
}

QRect PanelKMenu::sideImageRect()
{
    return QStyle::visualRect( layoutDirection(), rect(), QRect( frameWidth(), frameWidth(), sidePixmap.width(),
                                      height() - 2*frameWidth() ) );
}

void PanelKMenu::resizeEvent(QResizeEvent * e)
{
//    kDebug(1210) << "PanelKMenu::resizeEvent():" << endl;
//    kDebug(1210) << geometry().width() << ", " << geometry().height() << endl;

    PanelServiceMenu::resizeEvent(e);
#warning "KDE4: Qt4 doesn't seem to provide a way of doing this, will need different impl. for side image"
//    setFrameRect( QStyle::visualRect( layoutDirection(), rect(), QRect( sidePixmap.width(), 0,
//                                      width() - sidePixmap.width(), height() ) ) );
}

//Workaround Qt3.3.x sizing bug, by ensuring we're always wide enough.
void PanelKMenu::resize(int width, int height)
{
    width = qMax(width, maximumSize().width());
    PanelServiceMenu::resize(width, height);
}

QSize PanelKMenu::sizeHint() const
{
    QSize s = PanelServiceMenu::sizeHint();
//    kDebug(1210) << "PanelKMenu::sizeHint()" << endl;
//    kDebug(1210) << s.width() << ", " << s.height() << endl;
    return s;
}

void PanelKMenu::paintEvent(QPaintEvent * e)
{
    if (sidePixmap.isNull()) {
        PanelServiceMenu::paintEvent(e);
        return;
    }

    QPainter p(this);
    p.setClipRegion(e->region());

    QStyleOptionFrame frOpt;
    frOpt.init(this);
    frOpt.lineWidth    = frameWidth();
    frOpt.midLineWidth = 0;
    style()->drawPrimitive( QStyle::PE_FrameMenu, &frOpt, &p, this);

    QRect r = sideImageRect();
    r.setBottom( r.bottom() - sidePixmap.height() );
    if ( r.intersects( e->rect() ) )
    {
        p.drawTiledPixmap( r, sideTilePixmap );
    }

    r = sideImageRect();
    r.setTop( r.bottom() - sidePixmap.height() );
    if ( r.intersects( e->rect() ) )
    {
        QRect drawRect = r.intersect( e->rect() );
        QRect pixRect = drawRect;
        pixRect.translate( -r.left(), -r.top() );
        p.drawPixmap( drawRect.topLeft(), sidePixmap, pixRect );
    }

    PanelServiceMenu::paintEvent( e );
}

QMouseEvent PanelKMenu::translateMouseEvent( QMouseEvent* e )
{
    QRect side = sideImageRect();

    if ( !side.contains( e->pos() ) )
        return *e;

    QPoint newpos( e->pos() );
    QApplication::isRightToLeft() ?
        newpos.setX( newpos.x() - side.width() ) :
        newpos.setX( newpos.x() + side.width() );
    QPoint newglobal( e->globalPos() );
    QApplication::isRightToLeft() ?
        newglobal.setX( newpos.x() - side.width() ) :
        newglobal.setX( newpos.x() + side.width() );

    return QMouseEvent( e->type(), newpos, newglobal, e->button(), e->state() );
}

void PanelKMenu::mousePressEvent(QMouseEvent * e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    PanelServiceMenu::mousePressEvent( &newEvent );
}

void PanelKMenu::mouseReleaseEvent(QMouseEvent *e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    PanelServiceMenu::mouseReleaseEvent( &newEvent );
}

void PanelKMenu::mouseMoveEvent(QMouseEvent *e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    PanelServiceMenu::mouseMoveEvent( &newEvent );
}

void PanelKMenu::configChanged()
{
    RecentlyLaunchedApps::self().m_bNeedToUpdate = false;
    RecentlyLaunchedApps::self().configChanged();
    PanelServiceMenu::configChanged();
}

// create and fill "recent" section at first
void PanelKMenu::createRecentMenuItems()
{
    RecentlyLaunchedApps::self().init();
    RecentlyLaunchedApps::self().m_nNumMenuItems = 0;

    QStringList RecentApps;
    RecentlyLaunchedApps::self().getRecentApps(RecentApps);

    if (RecentApps.count() > 0)
    {
        bool bSeparator = KickerSettings::showMenuTitles();
        int nId = serviceMenuEndId() + 1;
        int nIndex = KickerSettings::showMenuTitles() ? 1 : 0;

        for (QList<QString>::iterator it =
             RecentApps.fromLast(); /*nop*/; --it)
        {
            KService::Ptr s = KService::serviceByDesktopPath(*it);
            if (!s)
            {
                RecentlyLaunchedApps::self().removeItem(*it);
            }
            else
            {
                if (bSeparator)
                {
                    bSeparator = false;
                    /*
                     FIXME: no more titles!
                    int id = insertTitle(
                        RecentlyLaunchedApps::self().caption(),
                        serviceMenuEndId(), 0);
                    setItemEnabled( id, false );
                    */
                    addSeparator();
                }
                insertMenuItem(s, nId++, nIndex);
                RecentlyLaunchedApps::self().m_nNumMenuItems++;
            }

            if (it == RecentApps.begin())
            {
                break;
            }
        }

        if (!KickerSettings::showMenuTitles())
        {
            insertSeparator(RecentlyLaunchedApps::self().m_nNumMenuItems);
        }
    }
}

void PanelKMenu::clearSubmenus()
{
    // we don't need to delete these on the way out since the libloader
    // handles them for us
    if (QApplication::closingDown())
    {
        return;
    }

    for (PopupMenuList::const_iterator it = dynamicSubMenus.constBegin();
            it != dynamicSubMenus.constEnd();
            ++it)
    {
        delete *it;
    }
    dynamicSubMenus.clear();

    PanelServiceMenu::clearSubmenus();
}

void PanelKMenu::updateRecent()
{
    if (!RecentlyLaunchedApps::self().m_bNeedToUpdate)
    {
        return;
    }

    RecentlyLaunchedApps::self().m_bNeedToUpdate = false;

    int nId = serviceMenuEndId() + 1;

    // remove previous items
    if (RecentlyLaunchedApps::self().m_nNumMenuItems > 0)
    {
        // -1 --> menu title
        int i = KickerSettings::showMenuTitles() ? -1 : 0;
        for (; i < RecentlyLaunchedApps::self().m_nNumMenuItems; i++)
        {
            removeItem(nId + i);
            entryMap_.remove(nId + i);
        }
        RecentlyLaunchedApps::self().m_nNumMenuItems = 0;

        if (!KickerSettings::showMenuTitles())
        {
            removeItemAt(0);
        }
    }

    // insert new items
    QStringList RecentApps;
    RecentlyLaunchedApps::self().getRecentApps(RecentApps);

    if (RecentApps.count() > 0)
    {
        bool bNeedSeparator = KickerSettings::showMenuTitles();
        for (QList<QString>::iterator it = RecentApps.fromLast();
             /*nop*/; --it)
        {
            KService::Ptr s = KService::serviceByDesktopPath(*it);
            if (!s)
            {
                RecentlyLaunchedApps::self().removeItem(*it);
            }
            else
            {
                if (bNeedSeparator)
                {
                    bNeedSeparator = false;
                    addSeparator();
                    /* FIXME: no more titles!
                    int id = insertTitle(
                        RecentlyLaunchedApps::self().caption(),
                        nId - 1, 0);
                    setItemEnabled( id, false );
                    */
                }
                insertMenuItem(s, nId++, KickerSettings::showMenuTitles() ?
                    1 : 0);
                RecentlyLaunchedApps::self().m_nNumMenuItems++;
            }

            if (it == RecentApps.begin())
                break;
        }

        if (!KickerSettings::showMenuTitles())
        {
            insertSeparator(RecentlyLaunchedApps::self().m_nNumMenuItems);
        }
    }
}

void PanelKMenu::clearRecentMenuItems()
{
    RecentlyLaunchedApps::self().clearRecentApps();
    RecentlyLaunchedApps::self().save();
    RecentlyLaunchedApps::self().m_bNeedToUpdate = true;
    updateRecent();
}


