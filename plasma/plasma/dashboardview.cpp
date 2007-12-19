/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "dashboardview.h"

#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTimer>

#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"
#include "plasma/appletbrowser.h"
#include "plasma/widgets/pushbutton.h"

#include "plasmaapp.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

DashboardView::DashboardView(int screen, QWidget *parent)
    : Plasma::View(screen, PlasmaApp::self()->corona(), parent),
      m_appletBrowserWidget(0),
      m_suppressShow(false),
      m_zoomIn(false),
      m_zoomOut(false)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags(Qt::FramelessWindowHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    setWindowState(Qt::WindowFullScreen);
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);

    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(screen));

    setDrawWallpaper(!PlasmaApp::hasComposite());
    hide();

    connect(scene(), SIGNAL(launchActivated()), SLOT(hideView()));
    connect(containment(), SIGNAL(showAddWidgets()), this, SLOT(showAppletBrowser()));
    Plasma::PushButton *tool = new Plasma::PushButton(i18n("Hide Dashboard"));
    tool->resize(tool->sizeHint());
    containment()->addToolBoxTool(tool, "hideDashboard");
    containment()->enableToolBoxTool("hideDashboard", false);
    connect(tool, SIGNAL(clicked()), this, SLOT(hideView()));
}

DashboardView::~DashboardView()
{
    delete m_appletBrowserWidget;
}

void DashboardView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        setDrawWallpaper(false);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(geometry(), QColor(0, 0, 0, 180));
    } else {
        setDrawWallpaper(true);
        Plasma::View::drawBackground(painter, rect);
    }
}

void DashboardView::showAppletBrowser()
{
    if (!m_appletBrowserWidget) {
        m_appletBrowserWidget = new Plasma::AppletBrowserWidget(containment(), true, this, Qt::FramelessWindowHint );
        m_appletBrowserWidget->setApplication();
        m_appletBrowserWidget->setWindowTitle(i18n("Add Widgets"));
        KWindowSystem::setState(m_appletBrowserWidget->winId(), NET::KeepAbove|NET::SkipTaskbar);
        //TODO: provide a nice unobtrusive way to access the browser
        m_appletBrowserWidget->move( 0, 0 );
    }

    m_appletBrowserWidget->setHidden(m_appletBrowserWidget->isVisible());
}

void DashboardView::appletBrowserDestroyed()
{
    m_appletBrowserWidget = 0;
}

void DashboardView::toggleVisibility()
{
    if (isHidden()) {
        if (m_suppressShow) {
            kDebug() << "DashboardView::toggleVisibility but show was suppressed";
            return;
        }

        m_zoomOut = containment()->isToolboxToolEnabled("zoomOut");
        m_zoomIn = containment()->isToolboxToolEnabled("zoomIn");
        containment()->enableToolBoxTool("hideDashboard", true);
        containment()->enableToolBoxTool("zoomOut", false);
        containment()->enableToolBoxTool("zoomIn", false);

        show();
        raise();

        m_suppressShow = true;
        QTimer::singleShot(SUPPRESS_SHOW_TIMEOUT, this, SLOT(suppressShowTimeout()));
    } else {
        if (m_zoomOut) {
            containment()->enableToolBoxTool("zoomOut", true);
        }

        if (m_zoomIn) {
            containment()->enableToolBoxTool("zoomOut", true);
        }

        hideView();
    }
}

void DashboardView::hideView()
{
    if (m_appletBrowserWidget) {
        m_appletBrowserWidget->hide();
    }

    containment()->enableToolBoxTool("hideDashboard", false);
    hide();
}

void DashboardView::suppressShowTimeout()
{
    kDebug() << "DashboardView::suppressShowTimeout";
    m_suppressShow = false;
}

void DashboardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideView();
    }

    Plasma::View::keyPressEvent(event);
}

#include "dashboardview.moc"

