/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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

// Own
#include "ui/itemdelegate.h"

// Qt
#include <QApplication>
#include <QFontMetrics>
#include <QIcon>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>

// KDE
#include <KColorUtils>
#include <KDebug>
#include <KGlobal>
#include <KGlobalSettings>

// plasma
#include <plasma/plasma.h>

using namespace Kickoff;

ItemDelegate::ItemDelegate()
{
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Plasma::Delegate::paint(painter, option, index);

    qreal freeSpace = -1;
    qreal usedSpace = -1;
    if (!index.data(DiskFreeSpaceRole).isNull()) {
        freeSpace = index.data(DiskFreeSpaceRole).value<int>()/1024.0/1024.0;
        usedSpace = index.data(DiskUsedSpaceRole).value<int>()/1024.0/1024.0;
    }


    // draw free space information (for drive icons)
    if (usedSpace >= 0) {
        painter->save();

        QRect emptyRect = rectAfterTitle(option, index);

        QSize barSize = QSize(qMin(emptyRect.width(), option.rect.width()/3), emptyRect.height());

        if (barSize.width() > 0) {
            // if the item view is gradually resized smaller or larger, make the bar fade out/in
            // as enough space for it becomes available
            if (barSize.width() < 20.0) {
                painter->setOpacity(barSize.width()/20.0);
            }

            QRectF spaceRect = QStyle::alignedRect(option.direction,
                                                   Qt::AlignRight, barSize, emptyRect);

            // add spacing between item text and free-space bar and tweak the position slightly
            // to give a sharp outline when drawn with anti-aliasing enabled
            spaceRect.translate(0.5, 0.5);

            QColor fillBrush = KColorUtils::mix(Qt::green, Qt::yellow, (usedSpace / (freeSpace+usedSpace)));
            QColor penColor = option.palette.mid().color();

            //if not mouse over
            if (!(option.state & (QStyle::State_Selected|QStyle::State_MouseOver|QStyle::State_HasFocus))) {
                fillBrush.setAlpha(75);
                penColor.setAlpha(75);
            }

            qreal width = (usedSpace / (freeSpace + usedSpace)) * spaceRect.width();
            painter->setPen(QPen(penColor, 0));
            painter->fillRect(QRectF(spaceRect.left(), spaceRect.top(), width, spaceRect.height()), fillBrush);
            painter->setBrush(QBrush(Qt::NoBrush));
            painter->drawRect(spaceRect);

            // -- Removed the free space text because it added too much 'visual noise' to the item
            //
                // some precision is lost here, but it is acceptible given that the disk-free bar
                // is only shown as a guide
                // QString freeSpaceString = KGlobal::locale()->formatByteSize(freeSpace*1024*1024*1024);
                // painter->drawText(spaceRect,Qt::AlignCenter,i18n("%1 free",freeSpaceString));
        }

        painter->restore();
    }

}

bool ItemDelegate::isVisible(const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());

    if (index.model()->hasChildren(index)) {
        int childCount = index.model()->rowCount(index);
        for (int i=0; i<childCount; ++i) {
            QModelIndex child = index.model()->index(i, 0, index);
            if (!child.data(UrlRole).isNull()) {
                return true;
            }
        }
        return false;
    }

    return !index.data(UrlRole).isNull();
}

