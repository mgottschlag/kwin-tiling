/***************************************************************************
 *   taskarea.cpp                                                          *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef TASKAREA_P_H
#define TASKAREA_P_H

#include <Plasma/Label>

namespace SystemTray
{

class HiddenTaskLabel : public Plasma::Label
{

    Q_OBJECT

public:
    HiddenTaskLabel(QGraphicsWidget *taskIcon, const QString &label, Plasma::ItemBackground *itemBackground, Plasma::Applet *applet, QGraphicsWidget *parent = 0)
        : Plasma::Label(parent),
          m_taskIcon(taskIcon),
          m_itemBackground(itemBackground),
          m_applet(applet)
    {
        taskIcon->setMaximumHeight(48);
        taskIcon->setMinimumHeight(24);
        taskIcon->setMinimumWidth(24);

        nativeWidget()->setIndent(6);
        setContentsMargins(6, 0, 0, 0);

        setWordWrap(false);
        setText(label);
        if (!itemBackground->scene()) {
            scene()->addItem(itemBackground);
        }
    }

public slots:
    void taskChanged(SystemTray::Task *task)
    {
        setText(task->name());
    }

protected:
    void takeItemBackgroundOwnership()
    {
        if (m_taskIcon) {
            QRectF totalRect = geometry().united(m_taskIcon.data()->geometry());
            totalRect.moveTopLeft(QPoint(0,0));
            totalRect = m_taskIcon.data()->mapToScene(totalRect).boundingRect();
            qreal left, top, right, bottom;
            m_itemBackground->getContentsMargins(&left, &top, &right, &bottom);
            totalRect.adjust(-left/2, -top/2, right/2, bottom/2);
            m_itemBackground->setTarget(totalRect);
            m_itemBackground->show();
        }
    }

    template<class T> void forwardEvent(T *event)
    {
        if (m_taskIcon) {
            QGraphicsWidget *item = m_taskIcon.data();
            QPointF delta = item->sceneBoundingRect().center() - event->scenePos();
            event->setScenePos(item->sceneBoundingRect().center());
            event->setScreenPos((event->screenPos() + delta).toPoint());
            if (dynamic_cast<QGraphicsSceneContextMenuEvent *>(event) &&
                qobject_cast<Plasma::Applet*>(item) &&
                m_applet->containment()) {
                event->setPos(m_applet->containment()->mapFromScene(event->scenePos()));
                scene()->sendEvent(m_applet->containment(), event);
            } else if (qobject_cast<Plasma::Applet*>(item)) {
                event->setPos(scene()->itemAt(event->scenePos())->mapFromScene(event->scenePos()));
                scene()->sendEvent(scene()->itemAt(event->scenePos()), event);
            } else {
                event->setPos(item->boundingRect().center());
                scene()->sendEvent(item, event);
            }
        }
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        forwardEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        forwardEvent(event);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        forwardEvent(event);
    }

    void wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        forwardEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent * event)
    {
        takeItemBackgroundOwnership();
        forwardEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
    {
        forwardEvent(event);
    }

    void hoverMoveEvent(QGraphicsSceneHoverEvent * event)
    {
        forwardEvent(event);
    }

private:
    QWeakPointer<QGraphicsWidget> m_taskIcon;
    Plasma::ItemBackground *m_itemBackground;
    Plasma::Applet *m_applet;
};

}

#endif
