/*  Copyright 2009 by Martin Klapetek <martin.klapetek@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "devicespaceinfodelegate.h"

#include <kcapacitybar.h>
#include <KLocale>
#include <KGlobal>
#include <KDebug>

DeviceSpaceInfoDelegate::DeviceSpaceInfoDelegate(QObject* parent)
    : Plasma::Delegate(parent),
    m_capacityBar(0)
{
    m_capacityBar = new KCapacityBar(KCapacityBar::DrawTextInline);
    m_capacityBar->setMaximumWidth(200);
    m_capacityBar->setMinimumWidth(200);
}

DeviceSpaceInfoDelegate::~DeviceSpaceInfoDelegate()
{
    delete m_capacityBar;
}

void DeviceSpaceInfoDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    Plasma::Delegate::paint(painter, option, index);

    QVariantList sizes = index.data(Qt::UserRole + 7).value<QVariantList>(); // DeviceFreeSpaceRole =  Qt::UserRole + 7
    qulonglong size = 0;
    if (sizes.count() == 2) {
        size = qulonglong(sizes[0].toULongLong());
        qulonglong freeSpace = qulonglong(sizes[1].toULongLong());
        qulonglong usedSpace = size - freeSpace;

	m_capacityBar->setText(i18nc("@info:status Free disk space", "%1  free", KGlobal::locale()->formatByteSize(freeSpace)));
        m_capacityBar->setUpdatesEnabled(false);
        m_capacityBar->setValue(size > 0 ? (usedSpace * 100) / size : 0);
        m_capacityBar->setUpdatesEnabled(true);
    }
    if (size) {
        m_capacityBar->drawCapacityBar(painter, QRect(QPoint(option.rect.x()+48, Plasma::Delegate::rectAfterTitle(option, index).bottom()-5),  QSize(120, 14)));
    }
    painter->restore();
}

QSize DeviceSpaceInfoDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QSize size = Plasma::Delegate::sizeHint(option, index);
    size.setHeight(size.height()+m_capacityBar->barHeight()+14); //+14 is for kcapacitybar padding
    return size;
}
