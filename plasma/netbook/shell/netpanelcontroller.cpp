/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#include "netpanelcontroller.h"
#include "netview.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QToolButton>
#include <QLayout>

#include <KIcon>
#include <KIconLoader>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ToolButton>
#include <Plasma/Svg>
#include <Plasma/WindowEffects>

#include <kephal/screens.h>

NetPanelController::NetPanelController(QWidget *parent, NetView *view, Plasma::Containment *containment)
   : Plasma::Dialog(parent),
     m_containment(containment),
     m_view(view),
     m_watched(0)
{
    hide();

    m_mainWidget = new QGraphicsWidget(containment);
    if (containment && containment->corona()) {
        containment->corona()->addOffscreenWidget(m_mainWidget);
    }

    m_layout = new QGraphicsLinearLayout(Qt::Horizontal, m_mainWidget);

    m_iconSvg = new Plasma::Svg(this);
    m_iconSvg->setImagePath("widgets/configuration-icons");
    m_iconSvg->setContainsMultipleImages(true);
    m_iconSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    m_moveButton = new Plasma::ToolButton(m_mainWidget);
    m_moveButton->nativeWidget()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_moveButton->setIcon(m_iconSvg->pixmap("move"));
    m_moveButton->setText(i18n("Screen edge"));
    m_moveButton->setCursor(Qt::SizeAllCursor);
    m_layout->addItem(m_moveButton);

    m_resizeButton = new Plasma::ToolButton(m_mainWidget);
    m_resizeButton->nativeWidget()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_resizeButton->setIcon(m_iconSvg->pixmap("size-vertical"));
    m_resizeButton->setText(i18n("Height"));
    m_layout->addItem(m_resizeButton);

    m_autoHideButton = new Plasma::ToolButton(m_mainWidget);
    m_autoHideButton->nativeWidget()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_autoHideButton->nativeWidget()->setCheckable(true);
    m_autoHideButton->setIcon(m_iconSvg->pixmap("collapse"));
    m_autoHideButton->setText(i18n("Auto Hide"));
    m_layout->addItem(m_autoHideButton);
    m_autoHideButton->nativeWidget()->setChecked(view->autoHide());
    connect(m_autoHideButton->nativeWidget(), SIGNAL(toggled(bool)), view, SLOT(setAutoHide(bool)));
    connect(containment, SIGNAL(geometryChanged()), this, SLOT(updatePosition()));

    m_moveButton->installEventFilter(this);
    m_resizeButton->installEventFilter(this);
    setGraphicsWidget(m_mainWidget);
    layout()->activate();
    m_layout->activate();
    m_mainWidget->resize(m_mainWidget->effectiveSizeHint(Qt::PreferredSize));
    updatePosition();
    show();
    Plasma::WindowEffects::slideWindow(this, containment->location());
    KWindowSystem::setState(winId(), NET::KeepAbove|NET::StaysOnTop);
}

NetPanelController::~NetPanelController()
{
    Plasma::WindowEffects::slideWindow(this, m_containment->location());
}

void NetPanelController::updatePosition()
{
    QRect viewGeometry(m_view->geometry());
    switch (m_containment->location()) {
    case Plasma::LeftEdge:
        move(viewGeometry.right(), viewGeometry.center().y() - size().height()/2);
        break;
    case Plasma::RightEdge:
        move(viewGeometry.left() - size().width(), viewGeometry.center().y() - size().height()/2);
        break;
    case Plasma::TopEdge:
        move(viewGeometry.center().x() - size().width()/2, viewGeometry.bottom());
        break;
    case Plasma::BottomEdge:
        move(viewGeometry.center().x() - size().width()/2, viewGeometry.top() - size().height());
        break;
    default:
        break;
    }

    updateFormFactor();
}

void NetPanelController::updateFormFactor()
{
    QRect viewGeometry(m_view->geometry());
    switch (m_containment->location()) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        m_layout->setOrientation(Qt::Vertical);
        m_resizeButton->setIcon(m_iconSvg->pixmap("size-horizontal"));
        m_resizeButton->setText(i18n("Width"));
        m_resizeButton->setCursor(Qt::SizeHorCursor);
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        m_layout->setOrientation(Qt::Horizontal);
        m_resizeButton->setIcon(m_iconSvg->pixmap("size-vertical"));
        m_resizeButton->setText(i18n("Height"));
        m_resizeButton->setCursor(Qt::SizeVerCursor);
        break;
    default:
        break;
    }
}

void NetPanelController::resizeEvent(QResizeEvent *e)
{
    updatePosition();
    Plasma::Dialog::resizeEvent(e);
}

bool NetPanelController::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        m_watched = qobject_cast<Plasma::ToolButton *>(watched);
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        m_watched = 0;
    } else if (watched == m_moveButton && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        QRect screenGeom = Kephal::ScreenUtils::screenGeometry(m_containment->screen());

        //only move when the mouse cursor is out of the controller to avoid an endless reposition cycle
        if (geometry().contains(me->screenPos())) {
            return false;
        }

        if (!screenGeom.contains(me->screenPos())) {
            //move panel to new screen if dragged there
            int targetScreen = Kephal::ScreenUtils::screenId(me->screenPos());
            //kDebug() << "Moving panel from screen" << containment()->screen() << "to screen" << targetScreen;
            m_containment->setScreen(targetScreen);
            return false;
        }

        //create a dead zone so you can go across the middle without having it hop to one side
        float dzFactor = 0.35;
        QPoint offset = QPoint(screenGeom.width()*dzFactor,screenGeom.height()*dzFactor);
        QRect deadzone = QRect(screenGeom.topLeft()+offset, screenGeom.bottomRight()-offset);
        if (deadzone.contains(me->screenPos())) {
            //kDebug() << "In the deadzone:" << deadzone;
            return false;
        }

        const Plasma::Location oldLocation = m_containment->location();
        Plasma::Location newLocation = oldLocation;
        float screenAspect = float(screenGeom.height())/screenGeom.width();

        /* Use diagonal lines so we get predictable behavior when moving the panel
         * y=topleft.y+(x-topleft.x)*aspectratio   topright < bottomleft
         * y=bottomleft.y-(x-topleft.x)*aspectratio   topleft < bottomright
         */
        if (me->screenPos().y() < screenGeom.y()+(me->screenPos().x()-screenGeom.x())*screenAspect) {
            if (me->screenPos().y() < screenGeom.bottomLeft().y()-(me->screenPos().x()-screenGeom.x())*screenAspect) {
                if (m_containment->location() == Plasma::TopEdge) {
                    return false;
                } else {
                    newLocation = Plasma::TopEdge;
                }
            } else if (m_containment->location() == Plasma::RightEdge) {
                    return false;
            } else {
                newLocation = Plasma::RightEdge;
            }
        } else {
            if (me->screenPos().y() < screenGeom.bottomLeft().y()-(me->screenPos().x()-screenGeom.x())*screenAspect) {
                if (m_containment->location() == Plasma::LeftEdge) {
                    return false;
                } else {
                    newLocation = Plasma::LeftEdge;
                }
            } else if(m_containment->location() == Plasma::BottomEdge) {
                    return false;
            } else {
                newLocation = Plasma::BottomEdge;
            }
        }

        m_containment->setLocation(newLocation);

    } else if (watched == m_resizeButton && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        QPointF deltaPos(me->screenPos() - me->lastScreenPos());

        m_containment->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        switch (m_containment->location()) {
        case Plasma::LeftEdge:

            m_containment->setMinimumWidth(qBound((qreal)KIconLoader::SizeSmall, m_containment->size().width() + deltaPos.x(), (qreal)(KIconLoader::SizeEnormous*2)));
            m_containment->setMaximumWidth(m_containment->minimumWidth());

            break;
        case Plasma::RightEdge:

            m_containment->setMinimumWidth(qBound((qreal)KIconLoader::SizeSmall, m_containment->size().width() - deltaPos.x(), (qreal)(KIconLoader::SizeEnormous*2)));
            m_containment->setMaximumWidth(m_containment->minimumWidth());
            break;
        case Plasma::TopEdge:

            m_containment->setMinimumHeight(qBound((qreal)KIconLoader::SizeSmall, m_containment->size().height() + deltaPos.y(), (qreal)(KIconLoader::SizeEnormous*2)));
            m_containment->setMaximumHeight(m_containment->minimumHeight());
            break;
        case Plasma::BottomEdge:

            m_containment->setMinimumHeight(qBound((qreal)KIconLoader::SizeSmall, m_containment->size().height() - deltaPos.y(), (qreal)(KIconLoader::SizeEnormous*2)));
            m_containment->setMaximumHeight(m_containment->minimumHeight());
            break;
        default:
            break;
        }
    }
    return Plasma::Dialog::eventFilter(watched, event);
}

#include "netpanelcontroller.moc"
