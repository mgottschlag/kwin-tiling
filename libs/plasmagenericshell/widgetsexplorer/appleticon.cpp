/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "appleticon.h"

#include <KIconLoader>
#include <KIcon>

AppletIconWidget::AppletIconWidget(PlasmaAppletItem *appletItem)
    : AbstractIcon(0),
      m_runningIcon("dialog-ok")
{
    setAppletItem(appletItem);
}

AppletIconWidget::~AppletIconWidget()
{
}

PlasmaAppletItem *AppletIconWidget::appletItem()
{
    return m_appletItem.data();
}

void AppletIconWidget::setAppletItem(PlasmaAppletItem *appletItem)
{
    if (m_appletItem) {
        QStandardItemModel *model = m_appletItem.data()->model();
        if (model) {
            disconnect(model, 0, this, 0);
        }
    }

    m_appletItem = appletItem;
    if (appletItem) {
        kDebug() << "Applet item!" << appletItem << appletItem->name() << appletItem->model();
        setName(appletItem->name());
        QStandardItemModel *model = appletItem->model();
        if (model) {
            connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChanged(QStandardItem*)));
        }
    }

    setDraggable(appletItem);
    update();
}

void AppletIconWidget::itemChanged(QStandardItem *item)
{
    if (item == m_appletItem.data()) {
        update();
    }
}

QPixmap AppletIconWidget::pixmap(const QSize &size)
{
    if (m_appletItem) {
        return appletItem()->icon().pixmap(size);
    }
    return QPixmap();
}

QMimeData* AppletIconWidget::mimeData()
{
    if (m_appletItem) {
        return m_appletItem.data()->mimeData();
    }
    return 0;
}

void AppletIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractIcon::paint(painter, option, widget);

    PlasmaAppletItem *appletItem = m_appletItem.data();
    if (!appletItem) {
        return;
    }

    const QRectF rect = contentsRect();
    const int width = rect.width();

    QRect iconRect(rect.x() + qMax(0, (width / 2) - (iconSize() / 2)), rect.y(), iconSize(), iconSize());

    if (appletItem->running() > 0) {
        QSize runningIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        painter->drawPixmap(iconRect.bottomLeft().x(), iconRect.bottomLeft().y() - runningIconSize.height(),
                            m_runningIcon.pixmap(runningIconSize));
    }

}


#include "appleticon.moc"

