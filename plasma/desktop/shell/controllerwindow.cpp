/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "controllerwindow.h"

#include <QApplication>
#include <QBoxLayout>
#include <QGraphicsView>
#include <QPainter>

#include <kwindowsystem.h>
#include <netwm.h>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>

#include "widgetsExplorer/widgetexplorer.h"

#include <kephal/screens.h>

ControllerWindow::ControllerWindow(QWidget* parent)
   : QWidget(parent),
     m_location(Plasma::Floating),
     m_layout(new QBoxLayout(QBoxLayout::TopToBottom, this)),
     m_background(new Plasma::FrameSvg(this)),
     m_containment(0),
     m_widgetExplorerView(0),
     m_widgetExplorer(0)
{
    Q_UNUSED(parent)
    setWindowFlags(Qt::FramelessWindowHint);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocus(Qt::ActiveWindowFocusReason);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_background->setImagePath("dialogs/background");
    m_background->setContainsMultipleImages(true);

    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(onActiveWindowChanged(WId)));
    connect(m_background, SIGNAL(repaintNeeded()), SLOT(backgroundChanged()));
}

ControllerWindow::~ControllerWindow()
{
    delete m_widgetExplorer;
    delete m_widgetExplorerView;
}

void ControllerWindow::backgroundChanged()
{
    Plasma::Location l = m_location;
    m_location = Plasma::Floating;
    setLocation(l);
    update();
}

void ControllerWindow::setContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    if (m_containment) {
        disconnect(m_containment, 0, this, 0);
    }

    m_containment = containment;

    if (m_widgetExplorer) {
        m_widgetExplorer->setContainment(m_containment);
    }
}

Plasma::Containment *ControllerWindow::containment() const
{
    return m_containment;
}

QSize ControllerWindow::sizeHint() const
{
    if (!m_containment) {
        return QWidget::sizeHint();
    }

    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(m_containment->screen());

    switch (m_location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        return QSize(QWidget::sizeHint().width(), screenGeom.height());
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        return QSize(screenGeom.width(), QWidget::sizeHint().height());
        break;
    }
}

void ControllerWindow::setLocation(const Plasma::Location &loc)
{
    if (m_location == loc || !m_containment) {
        return;
    }

    m_location = loc;
    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(m_containment->screen());

    switch (loc) {
    case Plasma::LeftEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::RightBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        m_layout->setContentsMargins(0, 0, m_background->marginSize(Plasma::RightMargin), 0);
        break;

    case Plasma::RightEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        m_layout->setContentsMargins(m_background->marginSize(Plasma::LeftMargin), 0, 0, 0);
        break;

    case Plasma::TopEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::BottomBorder);
        m_layout->setDirection(QBoxLayout::BottomToTop);
        m_layout->setContentsMargins(0, 0, 0, m_background->marginSize(Plasma::BottomMargin));
        break;

    case Plasma::BottomEdge:
    default:
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        m_layout->setContentsMargins(0, m_background->marginSize(Plasma::TopMargin), 0, 0);
        break;
    }

    if (m_widgetExplorer) {
        switch (loc) {
        case Plasma::LeftEdge:
        case Plasma::RightEdge:
            m_widgetExplorer->setOrientation(Qt::Vertical);
            break;
        case Plasma::TopEdge:
        case Plasma::BottomEdge:
            m_widgetExplorer->setOrientation(Qt::Horizontal);
            break;
        default:
            break;
        }
    }
}

Plasma::Location ControllerWindow::location() const
{
    return m_location;
}

Qt::Orientation ControllerWindow::orientation() const
{
    if (m_location == Plasma::TopEdge || m_location == Plasma::BottomEdge) {
        return Qt::Horizontal;
    }

    return Qt::Vertical;
}

void ControllerWindow::showWidgetExplorer()
{
    if (!m_containment) {
        return;
    }

    if (!m_widgetExplorerView) {
        m_widgetExplorerView = new QGraphicsView(this);
        m_widgetExplorerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_widgetExplorerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_widgetExplorerView->setStyleSheet("background: transparent; border: none;");

        m_widgetExplorerView->setScene(m_containment->corona());
        m_widgetExplorerView->installEventFilter(this);
        m_layout->addWidget(m_widgetExplorerView);
    }

    if (!m_widgetExplorer) {
        m_widgetExplorer = new Plasma::WidgetExplorer();
        m_widgetExplorer->setContainment(m_containment);
        m_widgetExplorer->populateWidgetList();
        m_widgetExplorer->resize(size());

        m_containment->corona()->addOffscreenWidget(m_widgetExplorer);
        m_widgetExplorerView->setSceneRect(m_widgetExplorer->geometry());

        m_widgetExplorer->installEventFilter(this);
    }

    m_widgetExplorer->setOrientation(orientation());

    if (orientation() == Qt::Horizontal) {
        resize(width(), m_widgetExplorer->preferredSize().height());
    } else {
        resize(m_widgetExplorer->preferredSize().width(), height());
    }

    m_widgetExplorer->show();
    // connect signals
}

bool ControllerWindow::isWidgetExplorerVisible() const
{
    return m_widgetExplorerView && m_widgetExplorerView->isVisible();
}

Plasma::FrameSvg *ControllerWindow::background() const
{
    return m_background;
}

void ControllerWindow::onActiveWindowChanged(WId id)
{
    Q_UNUSED(id)

    //if the active window isn't the plasma desktop and the widgets explorer is visible,
    //then close the panel controller
    if (QApplication::activeWindow() == 0 || (QApplication::activeWindow()->winId() != KWindowSystem::activeWindow())) {
        if (m_widgetExplorerView && m_widgetExplorerView->isVisible() && !isActiveWindow()) {
            close();
        }
    }
}

void ControllerWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source );

    m_background->resizeFrame(size());
    m_background->paintFrame(&painter);
}

void ControllerWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

bool ControllerWindow::eventFilter(QObject *watched, QEvent *event)
{
    //if widgetsExplorer moves or resizes, then the view has to adjust
    if ((watched == (QObject*)m_widgetExplorer) && (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        m_widgetExplorerView->setSceneRect(m_widgetExplorer->geometry());
    }

    //if the view resizes, then the widgetexplorer has to be resized
    if (watched == m_widgetExplorerView && event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        m_widgetExplorer->resize(resizeEvent->size());
    }

    return false;
}

#include "controllerwindow.moc"
