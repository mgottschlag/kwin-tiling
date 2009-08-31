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
    : Plasma::ItemBackground(parent)
{
}

void SelectionBar::acquireTarget()
{
    QList<QGraphicsItem *> selection = scene()->selectedItems();
    if (selection.isEmpty()) {
        setTargetItem(0);
        return;
    }

    kDebug() << "showing an item!";
    setVisible(true);
    setTargetItem(selection.first());
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

