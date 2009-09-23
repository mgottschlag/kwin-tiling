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

#include <QAction>
#include <QKeyEvent>
#include <QTimer>
#include <QToolButton>

#include <KWindowSystem>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Svg>

#include <kephal/screens.h>

#include "plasmaapp.h"
#include "plasma-shell-desktop.h"
#include "widgetsExplorer/widgetexplorer.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

class DashboardWidgetExplorer : public Plasma::WidgetExplorer
{
public:
    DashboardWidgetExplorer(QGraphicsWidget *parent)
        : Plasma::WidgetExplorer(parent)
    {

    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        painter->fillRect(option->rect, QColor(0, 0, 0, 160));
    }
};

DashboardView::DashboardView(Plasma::Containment *containment, Plasma::View *view)
    : Plasma::View(containment, 0),
      m_view(view),
      m_widgetExplorer(0),
      m_closeButton(new QToolButton(this)),
      m_suppressShow(false),
      m_zoomIn(false),
      m_zoomOut(false),
      m_init(false)
{
    //setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags(Qt::FramelessWindowHint);
    setWallpaperEnabled(!PlasmaApp::hasComposite());
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    setGeometry(Kephal::ScreenUtils::screenGeometry(containment->screen()));

    m_hideAction = new QAction(i18n("Hide Dashboard"), this);
    m_hideAction->setIcon(KIcon("preferences-desktop-display"));
    m_hideAction->setEnabled(false);
    containment->addToolBoxAction(m_hideAction);
    connect(m_hideAction, SIGNAL(triggered()), this, SLOT(hideView()));

    installEventFilter(this);

    QFont f = font();
    f.bold();
    const QFontMetrics fm(f);
    m_closeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_closeButton->resize(fm.height(), fm.height());
    m_closeButton->setIcon(KIcon("window-close"));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(hideView()));
    connect(scene(), SIGNAL(releaseVisualFocus()), SLOT(hideView()));
}

DashboardView::~DashboardView()
{
    delete m_widgetExplorer;
}

void DashboardView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        setWallpaperEnabled(false);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, QColor(0, 0, 0, 180));
    } else {
        setWallpaperEnabled(true);
        Plasma::View::drawBackground(painter, rect);
    }
}

void DashboardView::paintEvent(QPaintEvent *event)
{
    Plasma::View::paintEvent(event);

    // now draw a little label saying "this is your friendly neighbourhood dashboard"
    const QRect r = rect();
    const QString text = i18n("Widget Dashboard");
    QFont f = font();
    f.bold();
    const QFontMetrics fm(f);
    const int margin = 6;
    const int textWidth = fm.width(text);
    const QPoint centered(r.width() / 2 - textWidth / 2 - margin - margin / 2 - m_closeButton->width() / 2, r.y());
    const QRect boundingBox(centered, QSize(margin * 3 + textWidth + m_closeButton->width(), fm.height() + margin * 2));

    if (!viewport() || !event->rect().intersects(boundingBox)) {
        return;
    }

    m_closeButton->move(boundingBox.right() - 6 - m_closeButton->width(), boundingBox.top() + margin);

    QPainterPath box;
    box.moveTo(boundingBox.topLeft());
    box.lineTo(boundingBox.bottomLeft() + QPoint(0, -margin * 2));
    box.quadTo(boundingBox.bottomLeft(), boundingBox.bottomLeft() + QPoint(margin * 2, 0));
    box.lineTo(boundingBox.bottomRight() + QPoint(-margin * 2, 0));
    box.quadTo(boundingBox.bottomRight(), boundingBox.bottomRight() + QPoint(0, -margin * 2));
    box.lineTo(boundingBox.topRight());
    box.closeSubpath();

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(f);
    //kDebug() << "******************** painting from" << centered << boundingBox << rect() << event->rect();
    QColor highlight = palette().highlight().color();
    highlight.setAlphaF(0.7);
    painter.setPen(highlight.darker());
    painter.setBrush(highlight);
    painter.drawPath(box);
    painter.setPen(palette().highlightedText().color());
    painter.drawText(boundingBox.adjusted(margin, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, text);
}

void DashboardView::showWidgetExplorer()
{
    Plasma::Containment *c = containment();
    if (!c) {
        return;
    }

    if (m_widgetExplorer) {
        delete m_widgetExplorer;
    } else {
        m_widgetExplorer = new DashboardWidgetExplorer(c);
        m_widgetExplorer->setContainment(c);
        m_widgetExplorer->populateWidgetList();
        m_widgetExplorer->setMaximumWidth(width());
        m_widgetExplorer->adjustSize();
        m_widgetExplorer->setPos(0, c->geometry().height() - m_widgetExplorer->geometry().height());
        m_widgetExplorer->setZValue(1000000);
    }
}

bool DashboardView::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }

    return Plasma::View::event(event);
}

void DashboardView::toggleVisibility()
{
    showDashboard(isHidden() && containment());
}

void DashboardView::showDashboard(bool showDashboard)
{
    if (showDashboard) {
        if (m_suppressShow) {
            //kDebug() << "DashboardView::toggleVisibility but show was suppressed";
            return;
        }

        KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);
        setWindowState(Qt::WindowFullScreen);

        if (AppSettings::perVirtualDesktopViews()) {
            KWindowSystem::setOnDesktop(winId(), m_view->desktop()+1);
        } else {
            KWindowSystem::setOnAllDesktops(winId(), true);
        }

        QAction *action = containment()->action("zoom out");
        m_zoomOut = action ? action->isEnabled() : false;
        action = containment()->action("zoom in");
        m_zoomIn = action ? action->isEnabled() : false;

        m_hideAction->setEnabled(true);
        containment()->enableAction("zoom out", false);
        containment()->enableAction("zoom in", false);

        show();
        raise();

        m_suppressShow = true;
        QTimer::singleShot(SUPPRESS_SHOW_TIMEOUT, this, SLOT(suppressShowTimeout()));
    } else {
        hideView();
    }
}

void DashboardView::setContainment(Plasma::Containment *newContainment)
{
    if (!newContainment || (m_init && newContainment == containment())) {
        return;
    }

    m_init = true;

    Plasma::Containment *oldContainment = containment();
    if (oldContainment) {
        oldContainment->removeToolBoxAction(m_hideAction);
    }

    newContainment->addToolBoxAction(m_hideAction);

    if (isVisible()) {
        if (oldContainment) {
            disconnect(oldContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
            oldContainment->closeToolBox();
            oldContainment->enableAction("zoom out", m_zoomOut);
            oldContainment->enableAction("zoom in", m_zoomIn);
        }

        connect(newContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
        QAction *action = newContainment->action("zoom out");
        m_zoomOut = action ? action->isEnabled() : false;
        action = newContainment->action("zoom in");
        m_zoomIn = action ? action->isEnabled() : false;
        newContainment->enableAction("zoom out", false);
        newContainment->enableAction("zoom in", false);
    }

    if (m_widgetExplorer) {
        m_widgetExplorer->setContainment(newContainment);
    }

    View::setContainment(0); // we don't actually to mess with the screen settings
    View::setContainment(newContainment);
}

void DashboardView::hideView()
{
    delete m_widgetExplorer;

    if (containment()) {
        disconnect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));

        containment()->closeToolBox();
        containment()->enableAction("zoom out", m_zoomOut);
        containment()->enableAction("zoom in", m_zoomIn);
    }

    m_hideAction->setEnabled(false);
    hide();
}

void DashboardView::suppressShowTimeout()
{
    //kDebug() << "DashboardView::suppressShowTimeout";
    m_suppressShow = false;
}

void DashboardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideView();
        event->accept();
        return;
    }

    Plasma::View::keyPressEvent(event);
}

void DashboardView::showEvent(QShowEvent *event)
{
    KWindowSystem::setState(winId(), NET::SkipPager);
    if (containment()) {
        connect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
    }
    Plasma::View::showEvent(event);
}

#include "dashboardview.moc"

