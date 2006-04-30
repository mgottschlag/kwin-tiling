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

#include <qpainter.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <QMenu>
#include <qlayout.h>
#include <QButtonGroup>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QGridLayout>
#include <QList>
#include <QResizeEvent>

#include <dcopref.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <netwm.h>
#include <ktoolinvocation.h>
#include <kauthorized.h>

#include "utils.h"
#include "kickertip.h"
#include "kickerSettings.h"
#include "kshadowengine.h"
#include "kshadowsettings.h"
#include "taskmanager.h"

#include "pagerapplet.h"
#include "pagerapplet.moc"

#ifdef FocusOut
#undef FocusOut
#endif

const int knDesktopPreviewSize = 12;
const int knBtnSpacing = 1;

// The previews tend to have a 4/3 aspect ratio
static const int smallHeight = 32;
static const int smallWidth = 42;

// config menu id offsets
static const int rowOffset = 2000;
static const int labelOffset = 200;
static const int bgOffset = 300;

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
      KGlobal::locale()->insertCatalog("kminipagerapplet");
      return new KMiniPager(configFile, Plasma::Normal, 0, parent, "kminipagerapplet");
    }
}

KMiniPager::KMiniPager(const QString& configFile, Plasma::Type type, int actions,
                       QWidget *parent, const char *name)
    : KPanelApplet( configFile, type, actions, parent, name ),
      m_layout(0),
      m_shadowEngine(0),
      m_contextMenu(0),
      m_settings( new PagerSettings(sharedConfig()) )
{
    m_settings->readConfig();
    TaskManager::self()->trackGeometry();

    m_group = new QButtonGroup(this);
    m_group->setExclusive( true );

    setFont( KGlobalSettings::taskbarFont() );

    int scnum = QApplication::desktop()->screenNumber(this);
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    if (desk.width() <= 800)
    {
        m_settings->setPreview(false);
    }

    m_kwin = new KWinModule(this);
    m_activeWindow = m_kwin->activeWindow();
    m_curDesk = m_kwin->currentDesktop();

    if (m_curDesk == 0) // kwin not yet launched
    {
        m_curDesk = 1;
    }

    desktopLayoutOrientation = Qt::Horizontal;
    desktopLayoutX = -1;
    desktopLayoutY = -1;

    drawButtons();

    connect( m_kwin, SIGNAL( currentDesktopChanged(int)), SLOT( slotSetDesktop(int) ) );
    connect( m_kwin, SIGNAL( numberOfDesktopsChanged(int)), SLOT( slotSetDesktopCount(int) ) );
    connect( m_kwin, SIGNAL( activeWindowChanged(WId)), SLOT( slotActiveWindowChanged(WId) ) );
    connect( m_kwin, SIGNAL( windowAdded(WId) ), this, SLOT( slotWindowAdded(WId) ) );
    connect( m_kwin, SIGNAL( windowRemoved(WId) ), this, SLOT( slotWindowRemoved(WId) ) );
    connect( m_kwin, SIGNAL( windowChanged(WId,unsigned int) ), this, SLOT( slotWindowChanged(WId,unsigned int) ) );
    connect( m_kwin, SIGNAL( desktopNamesChanged() ), this, SLOT( slotDesktopNamesChanged() ) );
    connect( kapp, SIGNAL(backgroundChanged(int)), SLOT(slotBackgroundChanged(int)) );

    if (KAuthorized::authorizeKAction("kicker_rmb") &&
        KAuthorized::authorizeControlModule("kde-kcmtaskbar.desktop"))
    {
        m_contextMenu = new QMenu();
        connect(m_contextMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowContextMenu()));
        connect(m_contextMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
        setCustomMenu(m_contextMenu);
    }

    QList<WId> ids = m_kwin->windows();
    foreach ( WId id, ids )
    {
        slotWindowAdded( id );
    }

    slotSetDesktop( m_curDesk );
    updateLayout();
}

KMiniPager::~KMiniPager()
{
    KGlobal::locale()->removeCatalog("kminipagerapplet");
    delete m_contextMenu;
}

void KMiniPager::slotBackgroundChanged(int desk)
{
    if (m_kwin->numberOfDesktops() > static_cast<int>(m_desktops.count()))
    {
        slotSetDesktopCount( m_kwin->numberOfDesktops() );
    }

    m_desktops[desk - 1]->backgroundChanged();
}

void KMiniPager::slotSetDesktop(int desktop)
{
    if (m_kwin->numberOfDesktops() > static_cast<int>(m_desktops.count()))
    {
        slotSetDesktopCount( m_kwin->numberOfDesktops() );
    }

    if (desktop != KWin::currentDesktop())
    {
        // this can happen when the user clicks on a desktop,
        // holds down the key combo to switch desktops, lets the
        // mouse go but keeps the key combo held. the desktop will switch
        // back to the desktop associated with the key combo and then it
        // becomes a race condition between kwin's signal and the button's
        // signal. usually kwin wins.
        return;
    }

    m_curDesk = desktop;
    if (m_curDesk < 1)
    {
        m_curDesk = 1;
    }

    KMiniPagerButton* button = m_desktops[m_curDesk - 1];
    if (!button->isChecked())
    {
        button->toggle();
    }
}

void KMiniPager::slotButtonSelected( int desk )
{
    KWin::setCurrentDesktop( desk );
    slotSetDesktop( desk );
}

int KMiniPager::widthForHeight(int h) const
{
    if (orientation() == Qt::Vertical)
    {
        return width();
    }

    int deskNum = m_kwin->numberOfDesktops();
    int rowNum = m_settings->numberOfRows();
    if (rowNum == 0)
    {
        if (h <= 32 || deskNum <= 1)
        {
            rowNum = 1;
        }
        else
        {
            rowNum = 2;
        }
    }

    int deskCols = deskNum/rowNum;
    if(deskNum == 0 || deskNum % rowNum != 0)
        deskCols += 1;

    int nWd;
    if( m_settings->labelType() != PagerSettings::EnumLabelType::LabelName )
    {
        int bw = (h/rowNum) + 1;
        if ( desktopPreview() )
        {
            bw = (int) ( bw * (double) QApplication::desktop()->width() / QApplication::desktop()->height() );
        }

        nWd = ( deskCols * bw);
    }
    else
    {
        int bw = (h/rowNum) + 1;
        for (int i = 1; i <= deskNum; i++ )
        {
            int sw = fontMetrics().width( m_kwin->desktopName( i ) ) + 16;
            if ( sw > bw )
            {
                bw = sw;
            }
        }
        nWd = ( deskCols * bw );
    }

    return nWd;
}

int KMiniPager::heightForWidth(int w) const
{
    if (orientation() == Qt::Horizontal)
    {
        return height();
    }

    int deskNum = m_kwin->numberOfDesktops();
    int rowNum = m_settings->numberOfRows(); // actually these are columns now... oh well.
    if (rowNum == 0)
    {
        if (w <= 48 || deskNum == 1)
        {
            rowNum = 1;
        }
        else
        {
            rowNum = 2;
        }
    }

    int deskCols = deskNum/rowNum;
    if(deskNum == 0 || deskNum % rowNum != 0)
    {
        deskCols += 1;
    }

    int bh = (w/rowNum) + 1;
    if ( desktopPreview() )
    {
        bh = (int) ( bh *  (double) QApplication::desktop()->height() / QApplication::desktop()->width() );
    }
    else if ( m_settings->labelType() == PagerSettings::EnumLabelType::LabelName )
    {
       bh = fontMetrics().lineSpacing() + 8;
    }

    int nHg = ( deskCols * bh);

    return nHg;
}

void KMiniPager::updateDesktopLayout(int o, int x, int y)
{
    if ((desktopLayoutOrientation == o) &&
       (desktopLayoutX == x) &&
       (desktopLayoutY == y))
    {
      return;
    }

    if ( !(DCOPRef( "kwin", "KWinInterface" ).call( "setDesktopLayout(int, int, int) ", o, x, y) ).isValid() )
    {
        kDebug() << "KMiniPager: Call to KWinInterface::setDesktopLayout(int, int, int) failed" << endl;
        return;
    }

    desktopLayoutOrientation = o;
    desktopLayoutX = x;
    desktopLayoutY = y;
}

void KMiniPager::paintEvent(QPaintEvent *)
{
    // will be shown as 1 Pixel frame because of the grid's spacing...
    QPainter p(this);
    p.fillRect(rect(), (m_settings->backgroundType() == PagerSettings::EnumBackgroundType::BgTransparent) ?
               palette().color( QPalette::Background ) :
               palette().color( QPalette::Mid ).dark(140));
}

void KMiniPager::resizeEvent(QResizeEvent*)
{
    bool horiz = orientation() == Qt::Horizontal;

    int deskNum = m_desktops.count();
    int rowNum = m_settings->numberOfRows();
    if (rowNum == 0)
    {
        if (((horiz && height()<=32)||(!horiz && width()<=48)) || deskNum <= 1)
            rowNum = 1;
        else
            rowNum = 2;
    }

    int deskCols = deskNum/rowNum;
    if(deskNum == 0 || deskNum % rowNum != 0)
        deskCols += 1;

    if (m_layout)
    {
        delete m_layout;
        m_layout = 0;
    }

    int nDX, nDY;
    if (horiz)
    {
        nDX = rowNum;
        nDY = deskCols;
        updateDesktopLayout(Qt::Horizontal, -1, nDX);
    }
    else
    {
        nDX = deskCols;
        nDY = rowNum;
        updateDesktopLayout(Qt::Horizontal, nDY, -1);
    }

    // 1 pixel spacing.
    m_layout = new QGridLayout(this );
    m_layout->setSpacing( 1 );

    QList<KMiniPagerButton*>::Iterator it = m_desktops.begin();
    QList<KMiniPagerButton*>::Iterator itEnd = m_desktops.end();
    int c = 0,
        r = 0;
    while( it != itEnd ) {
        c = 0;
        while( (it != itEnd) && (c < nDY) ) {
            m_layout->addWidget( *it, r, c );
            ++it;
            ++c;
        }
        ++r;
    }

    m_layout->activate();
    updateGeometry();
}

void KMiniPager::wheelEvent( QWheelEvent* e )
{
    int newDesk;
    int desktops = KWin::numberOfDesktops();
    if (e->delta() < 0)
    {
        newDesk = m_curDesk % desktops + 1;
    }
    else
    {
        newDesk = (desktops + m_curDesk - 2) % desktops + 1;
    }

    slotButtonSelected(newDesk);
}

void KMiniPager::drawButtons()
{
    int deskNum = m_kwin->numberOfDesktops();
    KMiniPagerButton *desk;

    for ( int i = 1; i <= deskNum; ++i )
    {
        desk = new KMiniPagerButton( i, this );
        //btn->setOn(i == act);

        if ( m_settings->labelType() != PagerSettings::EnumLabelType::LabelName )
        {
            desk->setToolTip( desk->desktopName() );
        }

        m_desktops.append( desk );
        m_group->insert( desk );

        connect(desk, SIGNAL(buttonSelected(int)),
                SLOT(slotButtonSelected(int)) );
        connect(desk, SIGNAL(showMenu(const QPoint&, int )),
                SLOT(slotShowMenu(const QPoint&, int )) );

        desk->show();
    }
}

void KMiniPager::slotSetDesktopCount( int )
{
    QList<KMiniPagerButton*>::ConstIterator it;
    QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
    for( it = m_desktops.begin(); it != itEnd; ++it )
    {
        delete (*it);
    }
    m_desktops.clear();

    drawButtons();

    m_curDesk = m_kwin->currentDesktop();
    if ( m_curDesk == 0 )
    {
        m_curDesk = 1;
    }

    resizeEvent(0);
    updateLayout();
}

void KMiniPager::slotActiveWindowChanged( WId win )
{
    if (desktopPreview())
    {
        KWin::WindowInfo* inf1 = m_activeWindow ? info( m_activeWindow ) : NULL;
        KWin::WindowInfo* inf2 = win ? info( win ) : NULL;
        m_activeWindow = win;

        QList<KMiniPagerButton*>::ConstIterator it;
        QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for ( it = m_desktops.begin(); it != itEnd; ++it)
        {
            if ( ( inf1 && ( inf1->onAllDesktops() || (inf1->desktop() == (*it)->desktop()) ) ) ||
                 ( inf2 && ( inf2->onAllDesktops() || (inf2->desktop() == (*it)->desktop()) ) ) )
            {
                (*it)->windowsChanged();
            }
        }
    }
}

void KMiniPager::slotWindowAdded( WId win)
{
    if (desktopPreview())
    {
        KWin::WindowInfo* inf = info( win );

        if (inf->state() & NET::SkipPager)
        {
            return;
        }

        QList<KMiniPagerButton*>::ConstIterator it;
        QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for ( it = m_desktops.begin(); it != itEnd; ++it)
        {
            if ( inf->onAllDesktops() || inf->desktop() == (*it)->desktop() )
            {
                (*it)->windowsChanged();
            }
        }
    }
}

void KMiniPager::slotWindowRemoved(WId win)
{
    if (desktopPreview())
    {
        KWin::WindowInfo* inf = info(win);
        bool onAllDesktops = inf->onAllDesktops();
        bool skipPager = inf->state() & NET::SkipPager;
        int desktop = inf->desktop();

        if (win == m_activeWindow)
            m_activeWindow = 0;

        m_windows.remove((long) win);

        if (skipPager)
        {
            return;
        }

        QList<KMiniPagerButton*>::ConstIterator it;
        QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for (it = m_desktops.begin(); it != itEnd; ++it)
        {
            if (onAllDesktops || desktop == (*it)->desktop())
            {
                (*it)->windowsChanged();
            }
        }
    }
    else
    {
        m_windows.remove(win);
        return;
    }

}

void KMiniPager::slotWindowChanged( WId win , unsigned int properties )
{
    if ((properties & (NET::WMState | NET::XAWMState |
                       NET::WMDesktop | NET::WMGeometry)) == 0 &&
        (desktopPreview() && windowIcons() &&
         (properties & NET::WMIcon | NET::WMIconName | NET::WMVisibleIconName) == 0))
    {
        return;
    }

    if (desktopPreview())
    {
        KWin::WindowInfo* inf = m_windows[win];
        bool onAllDesktops = inf ? inf->onAllDesktops() : false;
        bool skipPager = inf ? inf->state() & NET::SkipPager : false;
        int desktop = inf ? inf->desktop() : 0;
        m_windows.remove((long) win);
        inf = info(win);

        if (inf->state() & NET::SkipPager || skipPager)
        {
            return;
        }

        QList<KMiniPagerButton*>::ConstIterator it;
        QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for ( it = m_desktops.begin(); it != itEnd; ++it)
        {
            if ( inf->onAllDesktops() || inf->desktop() == (*it)->desktop() || onAllDesktops || desktop == (*it)->desktop() )
            {
                (*it)->windowsChanged();
            }
        }
    }
    else
    {
        m_windows.remove(win);
        return;
    }
}

KWin::WindowInfo* KMiniPager::info( WId win )
{
    if (!m_windows[win] )
    {
        KWin::WindowInfo* info = new KWin::WindowInfo( win,
            NET::WMWindowType | NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMGeometry | NET::WMKDEFrameStrut, 0 );

        m_windows.insert( (long) win, info );
        return info;
    }

    return m_windows[win];
}

KShadowEngine* KMiniPager::shadowEngine()
{
    if (!m_shadowEngine)
    {
        KShadowSettings * shadset = new KShadowSettings();
        shadset->setOffsetX(0);
        shadset->setOffsetY(0);
        shadset->setThickness(1);
        shadset->setMaxOpacity(96);
        m_shadowEngine = new KShadowEngine(shadset);
    }

    return m_shadowEngine;
}

void KMiniPager::refresh()
{
    QList<KMiniPagerButton*>::ConstIterator it;
    QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
    for ( it = m_desktops.begin(); it != itEnd; ++it)
    {
        (*it)->update();
    }
}

void KMiniPager::aboutToShowContextMenu()
{
    m_contextMenu->clear();

    m_contextMenu->insertItem(SmallIcon("kpager"), i18n("&Launch Pager"), LaunchExtPager);
    m_contextMenu->insertSeparator();

    m_contextMenu->insertItem(i18n("&Rename Desktop \"%1\"",
                                    kwin()->desktopName(m_rmbDesk)), RenameDesktop);
    m_contextMenu->insertSeparator();

    KMenu* showMenu = new KMenu(m_contextMenu);
    showMenu->setCheckable(true);
    //FIXME: showMenu->insertTitle(i18n("Pager Layout"));

    QMenu* rowMenu = new QMenu(showMenu);
    rowMenu->setCheckable(true);
    rowMenu->insertItem(i18n("&Automatic"), 0 + rowOffset);
    rowMenu->insertItem(i18nc("one row or column", "&1"), 1 + rowOffset);
    rowMenu->insertItem(i18nc("two rows or columns", "&2"), 2 + rowOffset);
    rowMenu->insertItem( i18nc("three rows or columns", "&3"), 3 + rowOffset);
    connect(rowMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
    showMenu->insertItem((orientation()==Qt::Horizontal) ? i18n("&Rows"):
                                                       i18n("&Columns"),
                         rowMenu);

    showMenu->insertItem(i18n("&Window Thumbnails"), WindowThumbnails);
    showMenu->insertItem(i18n("&Window Icons"), WindowIcons);

    //FIXME: showMenu->insertTitle(i18n("Text Label"));
    showMenu->insertItem(i18n("Desktop N&umber"),
                         PagerSettings::EnumLabelType::LabelNumber + labelOffset);
    showMenu->insertItem(i18n("Desktop N&ame"),
                         PagerSettings::EnumLabelType::LabelName + labelOffset);
    showMenu->insertItem(i18n("N&o Label"),
                         PagerSettings::EnumLabelType::LabelNone + labelOffset);

    //FIXME:showMenu->insertTitle(i18n("Background"));
    showMenu->insertItem(i18n("&Elegant"),
                         PagerSettings::EnumBackgroundType::BgPlain + bgOffset);
    showMenu->insertItem(i18n("&Transparent"),
                         PagerSettings::EnumBackgroundType::BgTransparent + bgOffset);
    showMenu->insertItem(i18n("&Desktop Wallpaper"),
                         PagerSettings::EnumBackgroundType::BgLive + bgOffset);
    connect(showMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
    m_contextMenu->insertItem(i18n("&Pager Options"),showMenu);

    m_contextMenu->insertItem(SmallIcon("configure"),
                              i18n("&Configure Desktops..."),
                              ConfigureDesktops);

    rowMenu->setItemChecked(m_settings->numberOfRows() + rowOffset, true);
    m_contextMenu->setItemChecked(m_settings->labelType() + labelOffset, showMenu);
    m_contextMenu->setItemChecked(m_settings->backgroundType() + bgOffset, showMenu);

    m_contextMenu->setItemChecked(WindowThumbnails, m_settings->preview());
    m_contextMenu->setItemChecked(WindowIcons, m_settings->icons());
    m_contextMenu->setItemEnabled(WindowIcons, m_settings->preview());
    m_contextMenu->setItemEnabled(RenameDesktop,
                                  m_settings->labelType() ==
                                  PagerSettings::EnumLabelType::LabelName);
}

void KMiniPager::slotShowMenu(const QPoint& pos, int desktop)
{
    if (!m_contextMenu)
    {
        return;
    }

    m_rmbDesk = desktop;
    m_contextMenu->exec(pos);
    m_rmbDesk = -1;
}

void KMiniPager::contextMenuActivated(int result)
{
    if (result < 1)
    {
        return;
    }

    switch (result)
    {
        case LaunchExtPager:
            showPager();
            return;

        case ConfigureDesktops:
            KToolInvocation::startServiceByDesktopName("desktop");
            return;

        case RenameDesktop:
            m_desktops[(m_rmbDesk == -1) ? m_curDesk - 1 : m_rmbDesk - 1]->rename();
            return;
    }

    if (result >= rowOffset)
    {
        m_settings->setNumberOfRows(result - rowOffset);
        resizeEvent(0);
    }

    switch (result)
    {
        case WindowThumbnails:
            m_settings->setPreview(!m_settings->preview());
            break;

        case WindowIcons:
            m_settings->setIcons(!m_settings->icons());
            break;

        case PagerSettings::EnumBackgroundType::BgPlain + bgOffset:
            m_settings->setBackgroundType(PagerSettings::EnumBackgroundType::BgPlain);
            break;
        case PagerSettings::EnumBackgroundType::BgTransparent + bgOffset:
            m_settings->setBackgroundType(PagerSettings::EnumBackgroundType::BgTransparent);
            break;
        case PagerSettings::EnumBackgroundType::BgLive + bgOffset:
        {
            m_settings->setBackgroundType(PagerSettings::EnumBackgroundType::BgLive);
            QList<KMiniPagerButton*>::ConstIterator it;
            QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
            for ( it = m_desktops.begin(); it != itEnd; ++it)
            {
                (*it)->backgroundChanged();
            }
            break;
        }

        case PagerSettings::EnumLabelType::LabelNone + labelOffset:
            m_settings->setLabelType(PagerSettings::EnumLabelType::LabelNone);
            break;
        case PagerSettings::EnumLabelType::LabelNumber + labelOffset:
            m_settings->setLabelType(PagerSettings::EnumLabelType::LabelNumber);
            break;
        case PagerSettings::EnumLabelType::LabelName + labelOffset:
            m_settings->setLabelType(PagerSettings::EnumLabelType::LabelName);
            break;
    }

    m_settings->writeConfig();
    updateLayout();
}

void KMiniPager::slotDesktopNamesChanged()
{
    QList<KMiniPagerButton*>::ConstIterator it;
    QList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();

    for (it = m_desktops.begin(); it != itEnd; ++it)
    {
        (*it)->setToolTip( (*it)->desktopName());
    }
}

void KMiniPager::showPager()
{
    DCOPClient *dcop=kapp->dcopClient();

    if (dcop->isApplicationRegistered("kpager"))
    {
       showKPager(true);
    }
    else
    {
    // Let's run kpager if it isn't running
        connect( dcop, SIGNAL( applicationRegistered(const QByteArray &) ), this, SLOT(applicationRegistered(const QByteArray &)) );
        dcop->setNotifications(true);
        QString strAppPath(locate("exe", "kpager"));
        if (!strAppPath.isEmpty())
        {
            KProcess process;
            process << strAppPath;
            process << "--hidden";
            process.start(KProcess::DontCare);
        }
    }
}

void KMiniPager::showKPager(bool toggleShow)
{
    QPoint pt;
    switch ( position() )
    {
        case Plasma::Top:
            pt = mapToGlobal( QPoint(x(), y() + height()) );
            break;
        case Plasma::Left:
            pt = mapToGlobal( QPoint(x() + width(), y()) );
            break;
        case Plasma::Right:
        case Plasma::Bottom:
        default:
            pt=mapToGlobal( QPoint(x(), y()) );
    }


    if (toggleShow)
    {
        DCOPRef("kpager", "KPagerIface").send("toggleShow", pt.x(), pt.y());
    }
    else
    {
        DCOPRef("kpager", "KPagerIface").send("showAt", pt.x(), pt.y());
    }
}

void KMiniPager::applicationRegistered( const QByteArray  & appName )
{
    if (appName == "kpager")
    {
        disconnect( kapp->dcopClient(), SIGNAL( applicationRegistered(const QByteArray &) ),
                    this, SLOT(applicationRegistered(const QByteArray &)) );
        showKPager(false);
    }
}
