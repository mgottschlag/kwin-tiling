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

#include <Plasma/AbstractToolBox>
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Svg>

#include "desktopcorona.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"
#include "widgetsexplorer/widgetexplorer.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

class DashboardWidgetExplorer : public Plasma::WidgetExplorer
{
public:
    DashboardWidgetExplorer(QGraphicsWidget *parent)
        : Plasma::WidgetExplorer(parent)
    {
        connect(this, SIGNAL(closeClicked()), this, SLOT(deleteLater()));
        m_svg = new Plasma::FrameSvg(this);
        m_svg->setImagePath("widgets/frame");
        m_svg->setElementPrefix("raised");
        m_svg->setEnabledBorders(Plasma::FrameSvg::TopBorder);
        s_containmentsWithExplorer.insert(parent);
    }

    ~DashboardWidgetExplorer()
    {
        s_containmentsWithExplorer.remove(parentWidget());
    }

    static bool parentHasExplorer(QGraphicsWidget *parent)
    {
        return s_containmentsWithExplorer.contains(parent);
    }

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event)
    {
        m_svg->resizeFrame(event->newSize());
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)
        m_svg->paintFrame(painter);
    }

private:
    Plasma::FrameSvg *m_svg;
    static QSet<QGraphicsWidget *> s_containmentsWithExplorer;
};

QSet<QGraphicsWidget *> DashboardWidgetExplorer::s_containmentsWithExplorer;

DashboardView::DashboardView(Plasma::Containment *containment, Plasma::View *view)
    : Plasma::View(containment, 0),
      m_view(view),
      m_closeButton(new QToolButton(this)),
      m_suppressShow(false),
      m_zoomIn(false),
      m_zoomOut(false),
      m_init(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setWallpaperEnabled(!PlasmaApp::hasComposite());
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    setGeometry(PlasmaApp::self()->corona()->screenGeometry(containment->screen()));

    m_hideAction = new QAction(i18n("Hide Dashboard"), this);
    m_hideAction->setIcon(KIcon("preferences-desktop-display"));
    m_hideAction->setEnabled(false);
    m_hideAction->setData(Plasma::AbstractToolBox::DestructiveTool);
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
    connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), this, SLOT(compositingChanged(bool)));
}

DashboardView::~DashboardView()
{
    delete m_widgetExplorer.data();
}

void DashboardView::compositingChanged(bool changed)
{
    setWallpaperEnabled(!changed);
}

void DashboardView::drawBackground(QPainter *painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        const bool kwin = Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::Dashboard);
        // Note: KWin dahsboard plugin draws its own background for the dashboard
        painter->fillRect(rect, QColor(0, 0, 0, kwin ? 0 : 180));
    } else {
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
    if (DashboardWidgetExplorer::parentHasExplorer(c)) {
        return;
    }

    if (m_widgetExplorer) {
        delete m_widgetExplorer.data();
    } else {
        DashboardWidgetExplorer *widgetExplorer = new DashboardWidgetExplorer(c);
        m_widgetExplorer = widgetExplorer;
        widgetExplorer->installEventFilter(this);
        widgetExplorer->setContainment(c);
        widgetExplorer->setLocation(Plasma::BottomEdge);
        widgetExplorer->setIconSize(KIconLoader::SizeHuge);
        widgetExplorer->populateWidgetList();
        widgetExplorer->setMaximumWidth(width());
        widgetExplorer->adjustSize();
        widgetExplorer->resize(width(), widgetExplorer->size().height());
        widgetExplorer->setZValue(1000000);
        widgetExplorer->setFocus();
    }
}

bool DashboardView::eventFilter(QObject *watched, QEvent *event)
{
    if (containment() && (watched == (QObject*)m_widgetExplorer.data()) &&
        (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        Plasma::WidgetExplorer *widgetExplorer = m_widgetExplorer.data();
        widgetExplorer->setPos(0, containment()->geometry().height() - widgetExplorer->geometry().height());
    }

    return false;
}

void DashboardView::toggleVisibility()
{
    showDashboard(isHidden() && containment());
}

void DashboardView::showDashboard(bool showDashboard)
{
    if (showDashboard) {
        if (!containment()) {
            return;
        }

        if (m_suppressShow) {
            //kDebug() << "DashboardView::toggleVisibility but show was suppressed";
            return;
        }

        setWindowFlags(Qt::FramelessWindowHint);
        setWindowState(Qt::WindowFullScreen);

        // mark as dashboard
        Plasma::WindowEffects::markAsDashboard(winId());

        if (AppSettings::perVirtualDesktopViews()) {
            //kDebug() << "pvdv dashboard, setting" << winId() << "on desktop" << m_view->desktop() + 1;
            KWindowSystem::setOnDesktop(winId(), m_view->desktop() + 1);
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

	// dashboard is fullscreen and should never draw a shadow
        Plasma::WindowEffects::overrideShadow(winId(), true);

        // the order of the following lines is important; and mildly magical.
        KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);
        show();
        KWindowSystem::forceActiveWindow(winId());
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
        m_widgetExplorer.data()->setContainment(newContainment);
    }

    View::setContainment(0); // we don't actually to mess with the screen settings
    View::setContainment(newContainment);
}

void DashboardView::hideView()
{
    delete m_widgetExplorer.data();

    if (containment()) {
        disconnect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));

        containment()->closeToolBox();
        containment()->enableAction("zoom out", m_zoomOut);
        containment()->enableAction("zoom in", m_zoomIn);
    }

    m_hideAction->setEnabled(false);
    hide();
    emit dashboardClosed();
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

