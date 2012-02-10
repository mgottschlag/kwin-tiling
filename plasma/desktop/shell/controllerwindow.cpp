/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
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
#include <QPainter>

#include <kwindowsystem.h>
#include <netwm.h>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include <Plasma/WindowEffects>

#include "activitymanager/activitymanager.h"
#include "desktopcorona.h"
#include "panelview.h"
#include "plasmaapp.h"
#include "widgetsexplorer/widgetexplorer.h"

#include <kephal/screens.h>

ControllerWindow::ControllerWindow(QWidget* parent)
   : QWidget(parent),
     m_location(Plasma::Floating),
     m_layout(new QBoxLayout(QBoxLayout::TopToBottom, this)),
     m_background(new Plasma::FrameSvg(this)),
     m_screen(-1),
     m_view(0),
     m_watchedWidget(0),
     m_activityManager(0),
     m_widgetExplorer(0),
     m_graphicsWidget(0),
     m_ignoredWindowClosed(false)
{
    Q_UNUSED(parent)

    m_background->setImagePath("dialogs/background");
    m_background->setContainsMultipleImages(true);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
//    setFocus(Qt::ActiveWindowFocusReason);
    setLocation(Plasma::BottomEdge);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), Qt::transparent);
    setPalette(pal);

    Plasma::WindowEffects::overrideShadow(winId(), true);

    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(closeIfNotFocussed()));
    connect(m_background, SIGNAL(repaintNeeded()), SLOT(backgroundChanged()));
    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenResized(Kephal::Screen*,QSize,QSize)),
            this, SLOT(adjustAndSetMaxSize()));
    m_adjustViewTimer = new QTimer(this);
    m_adjustViewTimer->setSingleShot(true);
    connect(m_adjustViewTimer, SIGNAL(timeout()), this, SLOT(syncToGraphicsWidget()));
    adjustAndSetMaxSize();
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky);
}

ControllerWindow::~ControllerWindow()
{
    Plasma::Corona *corona = PlasmaApp::self()->corona(false);
    if (corona) {
        if (m_activityManager) {
            corona->removeOffscreenWidget(m_activityManager);
        }

        if (m_widgetExplorer) {
            corona->removeOffscreenWidget(m_widgetExplorer);
        }
    }

    delete m_activityManager;
    delete m_widgetExplorer;
    delete m_view;
}

void ControllerWindow::activate()
{
    KWindowSystem::activateWindow(winId());
}

void ControllerWindow::adjustAndSetMaxSize()
{
    QSize screenSize = PlasmaApp::self()->corona()->screenGeometry(PlasmaApp::self()->corona()->screenId(pos())).size();
    setMaximumSize(screenSize);
    adjustSize();
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
    if (containment == m_containment.data()) {
        return;
    }

    if (m_containment) {
        disconnect(m_containment.data(), 0, this, 0);
    }

    m_containment = containment;

    if (!containment) {
        return;
    }

    m_screen = containment->screen();

    if (m_widgetExplorer) {
        m_widgetExplorer->setContainment(containment);
    }
}

Plasma::Containment *ControllerWindow::containment() const
{
    return m_containment.data();
}

void ControllerWindow::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (m_graphicsWidget == widget) {
        return;
    }

    if (m_graphicsWidget) {
        m_graphicsWidget->removeEventFilter(this);
        if (m_graphicsWidget == m_widgetExplorer) {
            m_widgetExplorer->deleteLater();
            m_widgetExplorer = 0;
        } else if (m_graphicsWidget == m_activityManager) {
            m_activityManager->deleteLater();
            m_activityManager = 0;
        }
    }

    m_graphicsWidget = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        if (!m_view) {
            m_view = new QGraphicsView(this);
            m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_view->setFrameShape(QFrame::NoFrame);
            m_view->viewport()->setAutoFillBackground(false);
            layout()->addWidget(m_view);
        }

        m_view->setScene(widget->scene());

        //try to have the proper size -before- showing the dialog
        m_view->centerOn(widget);
        if (widget->layout()) {
            widget->layout()->activate();
        }
        static_cast<QGraphicsLayoutItem *>(widget)->updateGeometry();
        widget->resize(widget->size().expandedTo(widget->effectiveSizeHint(Qt::MinimumSize)));

        syncToGraphicsWidget();

        //adjustSizeTimer->start(150);

        widget->installEventFilter(this);
        adjustSize();

        bool moved = false;
        if (PlasmaApp::isPanelContainment(containment())) {
            // try to align it with the appropriate panel view
            foreach (PanelView * panel, PlasmaApp::self()->panelViews()) {
                if (panel->containment() == containment()) {
                    move(positionForPanelGeometry(panel->geometry()));
                    moved = true;
                    break;
                }
            }
        }

        if (!moved) {
            // set it to the bottom of the screen as we have no better hints to go by
            QRect geom = PlasmaApp::self()->corona()->availableScreenRect(m_screen);
            setGeometry(geom.x(), geom.bottom() - height(), geom.width(), height());
        }
    } else {
        delete m_view;
        m_view = 0;
    }
}

void ControllerWindow::syncToGraphicsWidget()
{
    m_adjustViewTimer->stop();
    if (m_view && m_graphicsWidget) {
        QSize prevSize = size();

        //set the sizehints correctly:
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);

        //Try to use the screenId directly from the containment, because it won't fail.
        //If we dont' have containment. try to get the screen by using QWidget::pos(), but it
        //may fail if syncToGraphicsWidget is called before a real position is set (so pos() will
        //just return 0x0, which may lead to the wrong screen
        const QRect screenRect = m_containment ? 
                                 PlasmaApp::self()->corona()->screenGeometry(m_containment.data()->screen()) :
                                 PlasmaApp::self()->corona()->screenGeometry(PlasmaApp::self()->corona()->screenId(pos()));

        QSize maxSize = KWindowSystem::workArea().intersect(screenRect).size();

        QSize windowSize;
        if (m_location == Plasma::LeftEdge || m_location == Plasma::RightEdge) {
            windowSize = QSize(qMin(int(m_graphicsWidget->size().width()) + left + right, maxSize.width()), maxSize.height());
            m_graphicsWidget->resize(m_graphicsWidget->size().width(), windowSize.height());
        } else {
            windowSize = QSize(maxSize.width(), qMin(int(m_graphicsWidget->size().height()) + top + bottom, maxSize.height()));
            m_graphicsWidget->resize(windowSize.width(), m_graphicsWidget->size().height());
        }

        setMinimumSize(windowSize);
        resize(windowSize);

        updateGeometry();

        //reposition and resize the view.
        //force a valid rect, otherwise it will take up the whole scene
        QRectF sceneRect(m_graphicsWidget->sceneBoundingRect());

        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        m_view->setSceneRect(sceneRect);

        m_view->centerOn(m_graphicsWidget);

    }
}

bool ControllerWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_graphicsWidget) {
        if (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove) {
            m_adjustViewTimer->start(150);
        }
    } else if (event->type() == QEvent::Close || event->type() == QEvent::Destroy) {
        m_ignoredWindowClosed = true;
    }

    return QWidget::eventFilter(watched, event);
}

void ControllerWindow::setLocation(const Plasma::Location &loc)
{
    if (m_location == loc) {
        return;
    }

    Plasma::WindowEffects::slideWindow(this, loc);

    m_location = loc;

    switch (loc) {
    case Plasma::LeftEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::RightBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        setContentsMargins(0, 0, m_background->marginSize(Plasma::RightMargin), 0);
        break;

    case Plasma::RightEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        setContentsMargins(m_background->marginSize(Plasma::LeftMargin), 0, 0, 0);
        break;

    case Plasma::TopEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::BottomBorder);
        m_layout->setDirection(QBoxLayout::BottomToTop);
        setContentsMargins(0, 0, 0, m_background->marginSize(Plasma::BottomMargin));
        break;

    case Plasma::BottomEdge:
    default:
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        setContentsMargins(0, m_background->marginSize(Plasma::TopMargin), 0, 0);
        break;
    }

    if (m_watchedWidget) {
        //FIXME maybe I should make these two inherit from something
        //or make orientation a slot.
        if (m_watchedWidget == (QGraphicsWidget*)m_widgetExplorer) {
            m_widgetExplorer->setLocation(location());
        } else {
            m_activityManager->setLocation(location());
        }
    }
}

QPoint ControllerWindow::positionForPanelGeometry(const QRect &panelGeom) const
{
    int screen;
    if (m_containment) {
        screen = m_containment.data()->screen();
    } else {
        //guess: 
        screen = PlasmaApp::self()->corona()->screenId(QCursor::pos());
    }

    QRect screenGeom = PlasmaApp::self()->corona()->screenGeometry(screen);

    switch (m_location) {
    case Plasma::LeftEdge:
        return QPoint(panelGeom.right(), screenGeom.top());
        break;
    case Plasma::RightEdge:
        return QPoint(panelGeom.left() - width(), screenGeom.top());
        break;
    case Plasma::TopEdge:
        return QPoint(screenGeom.left(), panelGeom.bottom());
        break;
    case Plasma::BottomEdge:
    default:
        return QPoint(screenGeom.left(), panelGeom.top() - height());
        break;
    }
}

Plasma::Location ControllerWindow::location() const
{
    return m_location;
}

Qt::Orientation ControllerWindow::orientation() const
{
    if (m_location == Plasma::LeftEdge || m_location == Plasma::RightEdge) {
        return Qt::Vertical;
    }

    return Qt::Horizontal;
}


void ControllerWindow::showWidgetExplorer()
{
    if (!m_containment) {
        return;
    }

    if (!m_widgetExplorer) {
        m_widgetExplorer = new Plasma::WidgetExplorer(location());
        m_watchedWidget = m_widgetExplorer;
        m_widgetExplorer->setContainment(m_containment.data());
        m_widgetExplorer->populateWidgetList();
        QAction *activityAction = new QAction(KIcon("preferences-activities"), i18n("Activities"), m_widgetExplorer);
        connect(activityAction, SIGNAL(triggered()), this, SLOT(showActivityManager()));
        m_widgetExplorer->addAction(activityAction);

        PlasmaApp::self()->corona()->addOffscreenWidget(m_widgetExplorer);
        m_widgetExplorer->show();

        if (orientation() == Qt::Horizontal) {
            m_widgetExplorer->resize(width(), m_widgetExplorer->size().height());
        } else {
            m_widgetExplorer->resize(m_widgetExplorer->size().width(), height());
        }

        setGraphicsWidget(m_widgetExplorer);

        connect(m_widgetExplorer, SIGNAL(closeClicked()), this, SLOT(close()));
    } else {
        m_widgetExplorer->setLocation(location());
        m_widgetExplorer->show();
        m_watchedWidget = m_widgetExplorer;
        setGraphicsWidget(m_widgetExplorer);
    }
    m_view->setFocus();
    m_widgetExplorer->setFocus();
}

bool ControllerWindow::showingWidgetExplorer() const
{
    return m_widgetExplorer;
}

void ControllerWindow::showActivityManager()
{
    if (!m_activityManager) {
        m_activityManager = new ActivityManager(location());
        m_watchedWidget = m_activityManager;

        PlasmaApp::self()->corona()->addOffscreenWidget(m_activityManager);
        m_activityManager->show();

        if (orientation() == Qt::Horizontal) {
            m_activityManager->resize(width(), m_activityManager->size().height());
        } else {
            m_activityManager->resize(m_activityManager->size().width(), height());
        }

        setGraphicsWidget(m_activityManager);

        connect(m_activityManager, SIGNAL(addWidgetsRequested()), this, SLOT(showWidgetExplorer()));
        connect(m_activityManager, SIGNAL(closeClicked()), this, SLOT(close()));
    } else {
        m_activityManager->setLocation(location());
        m_watchedWidget = m_activityManager;
        m_activityManager->show();
        setGraphicsWidget(m_activityManager);
    }
    m_view->setFocus();
    m_activityManager->setFlag(QGraphicsItem::ItemIsFocusable);
    m_activityManager->setFocus();
}

bool ControllerWindow::showingActivityManager() const
{
    return m_activityManager;
}

bool ControllerWindow::isControllerViewVisible() const
{
    return m_view && m_view->isVisible();
}

Plasma::FrameSvg *ControllerWindow::background() const
{
    return m_background;
}

int ControllerWindow::screen() const
{
    return m_screen;
}

void ControllerWindow::setScreen(int screen)
{
    m_screen = screen;
}

void ControllerWindow::closeIfNotFocussed()
{
    QWidget *widget = QApplication::activeWindow();
    if (!widget) {
        if (m_ignoredWindowClosed) {
            m_ignoredWindowClosed = false;
        } else {
            // single shot to work around Qt 4.8+ bug in event loop count in x11event handler
            QTimer::singleShot(0, this, SLOT(deleteLater()));
        }
    } else if (widget != this) {
        KWindowInfo info(widget->winId(), NET::WMWindowType);
        if (info.windowType(NET::DesktopMask | NET::DockMask | NET::PopupMenuMask) == -1) {
            // an unfortunate little hack to allow windows to be tagged in a way that they don't
            // close the controller
            bool shouldClose = true;
            QWidget *checkWidget = widget;
            while (checkWidget) {
                if (!checkWidget->property("DoNotCloseController").isNull()) {
                    shouldClose = false;
                    break;
                }

                checkWidget = checkWidget->parentWidget();
            }

            if (shouldClose) {
                // single shot to work around Qt 4.8+ bug in event loop count in x11event handler
                QTimer::singleShot(0, this, SLOT(deleteLater()));
            } else {
                // we need to watch to see when it closes to prevent closing the controller when 
                // this "don't close" window closes
                widget->installEventFilter(this);
            }
        }
    }
}

void ControllerWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source );

    m_background->paintFrame(&painter);
}

void ControllerWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

void ControllerWindow::resizeEvent(QResizeEvent * event)
{
    m_background->resizeFrame(size());

    Plasma::WindowEffects::enableBlurBehind(effectiveWinId(), true, m_background->mask());

    qDebug() << "ControllerWindow::resizeEvent" << event->oldSize();

    QWidget::resizeEvent(event);
    if (PlasmaApp::isPanelContainment(containment())) {
        // try to align it with the appropriate panel view
        foreach (PanelView * panel, PlasmaApp::self()->panelViews()) {
            if (panel->containment() == containment()) {
                move(positionForPanelGeometry(panel->geometry()));
                break;
            }
        }
    }
}

#include "controllerwindow.moc"
