/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "desktopview.h"

#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>
#include <KWindowSystem>
#include <NETRootInfo>
#include <KAction>
#include <KActionCollection>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Svg>
#include <Plasma/Wallpaper>
#include <Plasma/Theme>

#include <kephal/screens.h>

#include "dashboardview.h"
#include "desktopcorona.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"

#ifdef Q_WS_WIN
#include "windows.h"
#include "windef.h"
#include "wingdi.h"
#include "winuser.h"
#endif

DesktopView::DesktopView(Plasma::Containment *containment, int id, QWidget *parent)
    : Plasma::View(containment, id, parent),
      m_dashboard(0),
      m_dashboardFollowsDesktop(true),
      m_init(false),
      m_actions(shortcutActions(this))
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    //setCacheMode(QGraphicsView::CacheNone);

    /*FIXME: Work around for a (maybe) Qt bug:
     * QApplication::focusWidget() can't track focus change in QGraphicsProxyWidget
     *   wrapped normal widget (eg. QLineEdit), if the QGraphicsView has called 
     *   setFocusPolicy(Qt::NoFocus)
     * I've created a bug report to Qt Software.
     * There is also a simple reproduce program in case you're interested in this bug:
     *   ftp://fearee:public@public.sjtu.edu.cn/reproduce.tar.gz
     */

    //setFocusPolicy(Qt::NoFocus);
#ifdef Q_WS_WIN
    setWindowFlags(Qt::FramelessWindowHint);
    SetWindowPos(winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    HWND hwndDesktop = ::FindWindowW(L"Progman", NULL);
    SetParent(winId(), hwndDesktop);
#else
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#endif

    checkDesktopAffiliation();

    KWindowSystem::setType(winId(), NET::Desktop);
    lower();

    m_actions->addAssociatedWidget(this);
    QAction *action = m_actions->action("next");
    connect(action, SIGNAL(triggered()), this, SLOT(nextContainment()));
    action = m_actions->action("prev");
    connect(action, SIGNAL(triggered()), this, SLOT(previousContainment()));

    /*
    const int w = 25;
    QPixmap tile(w * 2, w * 2);
    tile.fill(palette().base().color());
    QPainter pt(&tile);
    QColor color = palette().mid().color();
    color.setAlphaF(.6);
    pt.fillRect(0, 0, w, w, color);
    pt.fillRect(w, w, w, w, color);
    pt.end();
    QBrush b(tile);
    setBackgroundBrush(tile);
    */

    KConfigGroup cg = config();
    const uint dashboardContainmentId = cg.readEntry("DashboardContainment", uint(0));
    m_dashboardFollowsDesktop = dashboardContainmentId == 0;

    // since Plasma::View has a delayed init we need to
    // put a delay also for this call in order to be sure
    // to have correct information set (e.g. screen())
    if (containment) {
        QRect geom = Kephal::ScreenUtils::screenGeometry(containment->screen());
        setGeometry(geom);
    }

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenResized(Kephal::Screen *, QSize, QSize)),
            this, SLOT(screenResized(Kephal::Screen *)));
    connect(screens, SIGNAL(screenMoved(Kephal::Screen *, QPoint, QPoint)),
            this, SLOT(screenMoved(Kephal::Screen *)));

    connect(this, SIGNAL(lostContainment()), SLOT(lostContainment()));
}

DesktopView::~DesktopView()
{
    delete m_dashboard;
}

void DesktopView::checkDesktopAffiliation()
{
    if (AppSettings::perVirtualDesktopViews()) {
        m_desktop = containment() ? containment()->desktop() + 1 : -1;
        kDebug() << "setting to desktop" << m_desktop;
        KWindowSystem::setOnDesktop(winId(), m_desktop);
    } else {
        m_desktop = -1;
        KWindowSystem::setOnAllDesktops(winId(), true);
    }
}

KActionCollection* DesktopView::shortcutActions(QObject *parent)
{
    KActionCollection *actions = new KActionCollection(parent);
    actions->setConfigGroup("Shortcuts-DesktopView");

    //FIXME should we have next/prev or up/down/left/right or what?
    KAction *action = actions->addAction("next");
    action->setText(i18n("Next Activity"));
    action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D, Qt::Key_Right));

    action = actions->addAction("prev");
    action->setText(i18n("Previous Activity"));
    action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D, Qt::Key_Left));

    actions->readSettings();
    return actions;
}

void DesktopView::updateShortcuts()
{
    m_actions->readSettings();
}

void DesktopView::toggleDashboard()
{
    //kDebug() << "toggling dashboard for screen" << screen() << "and destop" << desktop() <<
    //    (m_dashboard ? (m_dashboard->isVisible() ? "visible" : "hidden") : "non-existent");
    prepDashboard();
    if (m_dashboard) {
        m_dashboard->toggleVisibility();
    }
    //kDebug() << "toggling dashboard for screen" << screen() << "and destop" << desktop() << m_dashboard->isVisible();
}

void DesktopView::showDashboard(bool show)
{
    if (!show && (!m_dashboard || !m_dashboard->isVisible())) {
        return;
    }

    prepDashboard();
    if (m_dashboard) {
        m_dashboard->showDashboard(show);
    }
}

void DesktopView::prepDashboard()
{
    if (!m_dashboard) {
        if (!containment()) {
            return;
        }

        KConfigGroup cg = config();
        Plasma::Containment *dc = dashboardContainment();
        m_dashboardFollowsDesktop = dc == 0;
        if (dc) {
            dc->resize(size());
            dc->enableAction("remove", false);
        } else {
            dc = containment();
        }

        m_dashboard = new DashboardView(dc, this);
        connect(m_dashboard, SIGNAL(dashboardClosed()), this, SIGNAL(dashboardClosed()));
        m_dashboard->addActions(actions());
    }

    //If a separate dashboard is used we must use the screen of this containment instead of the dashboard one
    if (!m_dashboardFollowsDesktop && containment()) {
        m_dashboard->setGeometry(Kephal::ScreenUtils::screenGeometry(containment()->screen()));
    }
}

Plasma::Containment *DesktopView::dashboardContainment() const
{
    KConfigGroup cg = config();
    Plasma::Containment *dc = 0;
    const uint containmentId = cg.readEntry("DashboardContainment", uint(0));

    if (containmentId > 0) {
        foreach (Plasma::Containment *c, PlasmaApp::self()->corona()->containments()) {
            if (c->id() == containmentId) {
                dc = c;
                break;
            }
        }
    }

    return dc;
}

void DesktopView::setDashboardContainment(Plasma::Containment *containment)
{
    if (containment) {
        config().writeEntry("DashboardContainment", containment->id());
        if (m_dashboard) {
            m_dashboard->setContainment(containment);
        }
    } else {
        if (dashboardContainment()) {
            dashboardContainment()->destroy(false);
        }

        config().deleteEntry("DashboardContainment");
        if (m_dashboard) {
            m_dashboard->setContainment(View::containment());
        }
    }

    m_dashboardFollowsDesktop = containment == 0;
}

void DesktopView::screenResized(Kephal::Screen *s)
{
    if (s->id() == screen()) {
        kDebug() << screen();
        adjustSize();
    }
}

void DesktopView::screenMoved(Kephal::Screen *s)
{
    if (s->id() == screen()) {
        kDebug() << screen();
        adjustSize();
    }
}

void DesktopView::adjustSize()
{
    // adapt to screen resolution changes
    QRect geom = Kephal::ScreenUtils::screenGeometry(screen());
    kDebug() << "screen" << screen() << "geom" << geom;
    setGeometry(geom);
    if (containment()) {
        containment()->resize(geom.size());
        kDebug() << "Containment's geom after resize" << containment()->geometry();
    }

    if (m_dashboard) {
        m_dashboard->setGeometry(geom);
    }

    kDebug() << "Done" << screen() << geometry();
}

bool DesktopView::isDashboardVisible() const
{
    return m_dashboard && m_dashboard->isVisible();
}

bool DesktopView::dashboardFollowsDesktop() const
{
    return m_dashboardFollowsDesktop;
}

void DesktopView::setContainment(Plasma::Containment *containment)
{
    Plasma::Containment *oldContainment = this->containment();
    if (m_init && containment == oldContainment) {
        return;
    }

    m_init = true;

    if (m_dashboard && m_dashboardFollowsDesktop) {
        m_dashboard->setContainment(containment);
    }

    if (oldContainment) {
        disconnect(oldContainment, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(toolBoxOpened(bool)));
        disconnect(oldContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
        disconnect(oldContainment, SIGNAL(showActivityManager()), this, SLOT(showActivityManager()));
    }

    if (containment) {
        connect(containment, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(toolBoxOpened(bool)));
        connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
        connect(containment, SIGNAL(showActivityManager()), this, SLOT(showActivityManager()));
    }

    View::setContainment(containment);
}

void DesktopView::toolBoxOpened(bool open)
{
    if (isDashboardVisible()) {
        return;
    }

#ifndef Q_WS_WIN
    NETRootInfo info(QX11Info::display(), NET::Supported);
    if (!info.isSupported(NET::WM2ShowingDesktop)) {
        return;
    }

    if (open) {
        connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
                this, SLOT(showDesktopUntoggled(WId)));
    } else {
        disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
                   this, SLOT(showDesktopUntoggled(WId)));
    }

    info.setShowingDesktop(open);
#endif
}

void DesktopView::showWidgetExplorer()
{
    if (isDashboardVisible()) {
        return;
    }

    Plasma::Containment *c = containment();
    if (c) {
        PlasmaApp::self()->showWidgetExplorer(screen(), c);
    }
}

void DesktopView::showActivityManager()
{
    if (isDashboardVisible()) {
        return;
    }

    Plasma::Containment *c = containment();
    if (c) {
        PlasmaApp::self()->showActivityManager(screen(), c);
    }
}

void DesktopView::showDesktopUntoggled(WId id)
{
    if (isDashboardVisible()) {
        return;
    }

    Plasma::Containment *c = containment();
    if (c) {
        c->setToolBoxOpen(false);
    }

    KWindowSystem::activateWindow(id);
}

void DesktopView::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* newContainment)
{
    //kDebug() << "was:" << wasScreen << "is:" << isScreen << "my screen:" << screen() << "containment:" << (QObject*)newContainment << newContainment->activity() << "myself:" << (QObject*)this <<"containment desktop:"<<newContainment->desktop() << "my desktop:"<<containment()->desktop();
    if (PlasmaApp::isPanelContainment(newContainment)) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (wasScreen == screen() && this->containment() == newContainment) {
        setContainment(0);
    }

    if (isScreen > -1 && isScreen == screen() && (!AppSettings::perVirtualDesktopViews() || newContainment->desktop() == m_desktop - 1) ) {
        setContainment(newContainment);
    }
}

void DesktopView::nextContainment()
{
    QList<Plasma::Containment*> containments = PlasmaApp::self()->corona()->containments();
    int start = containments.indexOf(containment());
    int i = (start + 1) % containments.size();
    //FIXME this is a *horrible* way of choosing a "next" containment.
    while (i != start) {
        if (!PlasmaApp::isPanelContainment(containments.at(i)) &&
            containments.at(i)->screen() == -1) {
            break;
        }

        i = (i + 1) % containments.size();
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

void DesktopView::previousContainment()
{
    QList<Plasma::Containment*> containments = PlasmaApp::self()->corona()->containments();
    int start = containments.indexOf(containment());
    //fun fact: in c++, (-1 % foo) == -1
    int i = start - 1;
    if (i < 0) {
        i += containments.size();
    }

    //FIXME this is a *horrible* way of choosing a "previous" containment.
    while (i != start) {
        if (!PlasmaApp::isPanelContainment(containments.at(i)) &&
            containments.at(i)->screen() == -1) {
            break;
        }

        if (--i < 0) {
            i += containments.size();
        }
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

void DesktopView::lostContainment()
{
    QTimer::singleShot(0, this, SLOT(grabContainment()));
}

void DesktopView::grabContainment()
{
    kDebug() << "trying to find a containment @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";

    DesktopCorona *corona = qobject_cast<DesktopCorona*>(scene());
    if (!corona) {
        kDebug() << "no corona :(";
        return;
    }

    corona->addDesktopContainment(screen(), desktop());
    kDebug() << "success?" << (containment() != 0);
}

#include "desktopview.moc"

