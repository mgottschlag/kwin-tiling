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

#include <KIcon>
#include <KIconLoader>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ToolButton>
#include <Plasma/Svg>

NetPanelController::NetPanelController(QWidget *parent, NetView *view, Plasma::Containment *containment)
   : Plasma::Dialog(parent),
     m_containment(containment),
     m_watched(0)
{
    m_mainWidget = new QGraphicsWidget(containment);
    if (containment && containment->corona()) {
        containment->corona()->addOffscreenWidget(m_mainWidget);
    }

    m_layout = new QGraphicsLinearLayout(m_mainWidget);

    m_iconSvg = new Plasma::Svg(this);
    m_iconSvg->setImagePath("widgets/configuration-icons");
    m_iconSvg->setContainsMultipleImages(true);
    m_iconSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    /*m_moveButton = new Plasma::ToolButton(m_mainWidget);
    m_moveButton->nativeWidget()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_moveButton->setIcon(m_iconSvg->pixmap("move"));
    m_moveButton->setText(i18n("Screen edge"));
    m_layout->addItem(m_moveButton);*/

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

    //m_moveButton->installEventFilter(this);
    m_resizeButton->installEventFilter(this);
    setGraphicsWidget(m_mainWidget);
    show();
}

NetPanelController::~NetPanelController()
{
}

bool NetPanelController::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        m_watched = qobject_cast<Plasma::ToolButton *>(watched);
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        m_watched = 0;
    } else if (watched == m_moveButton && event->type() == QEvent::GraphicsSceneMouseMove) {
        
    } else if (watched == m_resizeButton && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        QPointF deltaPos(me->screenPos() - me->lastScreenPos());

        m_containment->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        switch (m_containment->location()) {
        case Plasma::LeftEdge:
            if ((deltaPos.x() > 0 && m_containment->size().width() >= KIconLoader::SizeEnormous) || (deltaPos.x() < 0 && m_containment->size().width() <= KIconLoader::SizeSmall)) {
                break;
            }

            m_containment->setMinimumHeight(m_containment->size().width() + deltaPos.x());
            m_containment->setMaximumHeight(m_containment->minimumHeight());
            move((pos() + QPointF(deltaPos.x(), 0)).toPoint());
            break;
        case Plasma::RightEdge:
            if ((deltaPos.x() < 0 && m_containment->size().width() >= KIconLoader::SizeEnormous) || (deltaPos.x() > 0 && m_containment->size().width() <= KIconLoader::SizeSmall)) {
                break;
            }

            m_containment->setMinimumHeight(m_containment->size().width() - deltaPos.x());
            m_containment->setMaximumHeight(m_containment->minimumHeight());
            move((pos() + QPointF(deltaPos.x(), 0)).toPoint());
            break;
        case Plasma::TopEdge:
            if ((deltaPos.y() > 0 && m_containment->size().height() >= KIconLoader::SizeEnormous) || (deltaPos.y() < 0 && m_containment->size().height() <= KIconLoader::SizeSmall)) {
                break;
            }

            m_containment->setMinimumHeight(m_containment->size().height() + deltaPos.y());
            m_containment->setMaximumHeight(m_containment->minimumHeight());
            move((pos() + QPointF(0, deltaPos.y())).toPoint());
            break;
        case Plasma::BottomEdge:
            if ((deltaPos.y() < 0 && m_containment->size().height() >= KIconLoader::SizeEnormous) || (deltaPos.y() > 0 && m_containment->size().height() <= KIconLoader::SizeSmall)) {
                break;
            }

            m_containment->setMinimumHeight(m_containment->size().height() - deltaPos.y());
            m_containment->setMaximumHeight(m_containment->minimumHeight());
            move((pos() + QPointF(0, deltaPos.y())).toPoint());
            break;
        default:
            break;
        }
    }
    return Plasma::Dialog::eventFilter(watched, event);
}

