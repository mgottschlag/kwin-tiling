/***************************************************************************
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>                        *
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

#include "selectionbar.h"

#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QTimer>

#include <KDebug>

#include <Plasma/Animator>
#include <Plasma/FrameSvg>

#include "resultitem.h"

SelectionBar::SelectionBar(QGraphicsWidget *parent)
    : Plasma::ItemBackground(parent),
      m_hideTimer(new QTimer(this))
{
    // the hide timer is necessary because when calling QGraphicsScene::setFocusItem, 
    // the selection is cleared first, then set. this causes acquireTarget to get called
    // multiple times, first with no selection. however, when the selection does go away
    // permanently, we do want to hide.
    m_hideTimer->setInterval(0);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(actuallyHide()));
}

void SelectionBar::acquireTarget()
{
    QList<QGraphicsItem *> selection = scene()->selectedItems();
    if (selection.isEmpty()) {
        m_hideTimer->start();
        return;
    }

    m_hideTimer->stop();
    //kDebug() << "showing an item!";
    setTargetItem(selection.first());
    setVisible(true);
}

void SelectionBar::actuallyHide()
{
    hide();
}

QVariant SelectionBar::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
        case ItemSceneChange: {
            if (scene()) {
                disconnect(scene(), SIGNAL(selectionChanged()), this, SLOT(acquireTarget()));
            }

            QGraphicsScene *newScene = value.value<QGraphicsScene*>();
            if (newScene) {
                connect(newScene, SIGNAL(selectionChanged()), this, SLOT(acquireTarget()));
            }
        }
        break;

        default:
        break;
    }

    return QGraphicsWidget::itemChange(change, value);
}

#include <selectionbar.moc>

