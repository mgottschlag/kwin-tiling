/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

#include <QtDebug>

// KDE
#include <KColorUtils>
#include <KGlobal>
#include <KGlobalSettings>

using namespace Kickoff;

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool hover = option.state & (QStyle::State_Selected|QStyle::State_MouseOver|QStyle::State_HasFocus);
    QRect decorationRect = QStyle::alignedRect(option.direction,
                                               option.decorationPosition == QStyleOptionViewItem::Left ? Qt::AlignLeft : Qt::AlignRight,
                                               option.decorationSize,
                                               option.rect);
    QSize textSize(option.rect.width()-decorationRect.width()-ICON_TEXT_MARGIN,
                   option.rect.height());

    qreal freeSpace = -1;
    qreal usedSpace = -1;
    if (!index.data(DiskFreeSpaceRole).isNull()) {
        freeSpace = index.data(DiskFreeSpaceRole).value<int>()/1024.0/1024.0;
        usedSpace = index.data(DiskUsedSpaceRole).value<int>()/1024.0/1024.0;
    }

    Qt::Alignment textAlignment = option.decorationAlignment & Qt::AlignRight ? Qt::AlignLeft : Qt::AlignRight;

    QRect textRect = QStyle::alignedRect(option.direction,
                                         textAlignment,
                                         textSize,
                                         option.rect);
    QString titleText = index.data(Qt::DisplayRole).value<QString>();
    QString subTitleText = index.data(SubTitleRole).value<QString>();

    QRect titleRect = textRect;

    if (!subTitleText.isEmpty()) {
        titleRect.setHeight(titleRect.height()/2);
    }

    QRect subTitleRect = titleRect;
    subTitleRect.translate(0,subTitleRect.height());

    // draw background on hover
    if (hover) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor backgroundColor = option.palette.color(QPalette::Highlight);
        // use a slightly translucent version of the palette's highlight color
        // for the background
        backgroundColor.setAlphaF(0.5);
        painter->setBrush(QBrush(backgroundColor));
        painter->drawPath(roundedRectangle(option.rect,5));
        painter->restore();
    }

    // draw icon
    QIcon decorationIcon = index.data(Qt::DecorationRole).value<QIcon>();
    decorationIcon.paint(painter, decorationRect, option.decorationAlignment);

    painter->save();
    
    // draw title and sub-title 
    QFont titleFont(option.font);
    painter->setFont(titleFont);
    painter->drawText(titleRect,Qt::AlignLeft|Qt::AlignVCenter,titleText);

    QFont subTitleFont = option.font;
    subTitleFont.setPointSize(qMax(subTitleFont.pointSize()-2,
                                   KGlobalSettings::smallestReadableFont().pointSize()));

    if (!hover) {
        painter->setPen(QPen(option.palette.mid(),0));
    }

    painter->setFont(subTitleFont);

    painter->drawText(subTitleRect,Qt::AlignLeft|Qt::AlignVCenter,subTitleText);
    painter->restore();

    // draw free space information (for drive icons)
    if (usedSpace >= 0) {
        painter->save();

        QFontMetrics titleMetrics(option.font);
        QFontMetrics subTitleMetrics(subTitleFont);

        QSize spaceSize = option.rect.size();
        spaceSize.rwidth() /= 3;
        spaceSize.rheight() -= 20;

        // check if there is enough space to draw the bar
        qreal actualTextWidth = qMax(titleMetrics.width(titleText),subTitleMetrics.width(subTitleText));
        qreal textBarGap = (titleRect.width() - actualTextWidth) - (spaceSize.width() + ICON_TEXT_MARGIN);
        if (textBarGap > 0) {

            // if the item view is gradually resized smaller or larger, make the bar fade out/in 
            // as enough space for it becomes available
            if (textBarGap < 20.0) {
                painter->setOpacity(textBarGap/20.0);
            }

            QRectF spaceRect = QStyle::alignedRect(option.direction,
                                                   Qt::AlignVCenter|Qt::AlignRight,spaceSize,option.rect);

            // add spacing between item text and free-space bar and tweak the position slightly
            // to give a shart outline when drawn with anti-aliasing enabled
            spaceRect.translate(-ICON_TEXT_MARGIN+0.5,0.5);

            QBrush fillBrush = KColorUtils::mix(Qt::green,Qt::yellow,(usedSpace/(freeSpace+usedSpace))); 

            qreal width = ( usedSpace / (freeSpace+usedSpace) ) * spaceRect.width();
            painter->setPen(QPen(option.palette.mid(),0));
            painter->fillRect(QRectF(spaceRect.left(),spaceRect.top(),width,spaceRect.height()),fillBrush); 
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

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem& option,const QModelIndex& index) const
{
    QSize size = option.decorationSize;
    
    QFontMetrics metrics(option.font);
    size.rwidth() += ICON_TEXT_MARGIN;
    size.rwidth() += qMax(metrics.width(index.data(Qt::DisplayRole).value<QString>()),
                          metrics.width(index.data(SubTitleRole).value<QString>()));
    size.rheight() = qMax(size.height(),2*metrics.height()+metrics.lineSpacing());

    return size;
}

bool ItemDelegate::isVisible(const QModelIndex& index) const 
{
    Q_ASSERT(index.isValid());

    return index.model()->hasChildren(index) || !index.data(UrlRole).isNull();
}

// Taken from kdelibs/kio/kio/kfileitemdelegate.cpp
QPainterPath ItemDelegate::roundedRectangle(const QRectF& rect, qreal radius) const
{
    QPainterPath path(QPointF(rect.left(), rect.top() + radius));
    path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());         // Top left corner
    path.lineTo(rect.right() - radius, rect.top());                                 // Top side
    path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + radius);       // Top right corner
    path.lineTo(rect.right(), rect.bottom() - radius);                              // Right side
    path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom()); // Bottom right corner
    path.lineTo(rect.left() + radius, rect.bottom());                               // Bottom side
    path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);   // Bottom left corner
    path.closeSubpath();

    return path;
}
