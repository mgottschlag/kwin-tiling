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
    const bool hover = option.state & (QStyle::State_Selected|QStyle::State_MouseOver|QStyle::State_HasFocus);
    QRect contentRect = option.rect;
    contentRect.setBottom(contentRect.bottom() - 1);
    QRect decorationRect = QStyle::alignedRect(option.direction,
                                               option.decorationPosition == QStyleOptionViewItem::Left ? Qt::AlignLeft : Qt::AlignRight,
                                               option.decorationSize,
                                               contentRect);
    QSize textSize(option.rect.width() - decorationRect.width() - ICON_TEXT_MARGIN,
                   option.rect.height() - 2);

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
                                         contentRect.adjusted(0, 2, 0, 0));
    QString titleText = index.data(Qt::DisplayRole).value<QString>();
    QString subTitleText = index.data(SubTitleRole).value<QString>();
    bool uniqueTitle = !index.data(SubTitleMandatoryRole).value<bool>();// true;
    if (uniqueTitle) {
        QModelIndex sib = index.sibling(index.row() + 1, index.column());
        if (sib.isValid()) {
            uniqueTitle = sib.data(Qt::DisplayRole).value<QString>() != titleText;
        }

        if (uniqueTitle) {
            sib = index.sibling(index.row() + -1, index.column());
            if (sib.isValid()) {
                uniqueTitle = sib.data(Qt::DisplayRole).value<QString>() != titleText;
            }
        }
    }

    QRect titleRect = textRect;

    if (subTitleText.isEmpty()) {
        subTitleText = " ";
    }

    titleRect.setHeight(titleRect.height() / 2);
    QRect subTitleRect = titleRect;
    subTitleRect.translate(0, subTitleRect.height());
    QFont subTitleFont = fontForSubTitle(option.font);

    QFont titleFont(option.font);

    if (hover) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor backgroundColor = option.palette.color(QPalette::Highlight);
        QFontMetrics titleMetrics(titleFont);
        QFontMetrics subTitleMetrics(subTitleFont);
        QRect textAreaRect = contentRect;
        int actualTextWidth = qMax(titleMetrics.width(titleText), subTitleMetrics.width(subTitleText));
        textAreaRect.adjust(decorationRect.width() + ICON_TEXT_MARGIN - 3,
                            0,
                            -(titleRect.width() - actualTextWidth) + 3,
                            1);
        // use a slightly translucent version of the palette's highlight color
        // for the background
        backgroundColor.setAlphaF(0.5);
        painter->setBrush(QBrush(backgroundColor));
        painter->drawPath(Plasma::roundedRectangle(textAreaRect, 5));
        painter->restore();
    }

    // draw icon
    QIcon decorationIcon = index.data(Qt::DecorationRole).value<QIcon>();
    decorationIcon.paint(painter, decorationRect, option.decorationAlignment);

    painter->save();

    // draw title
    painter->setFont(titleFont);
    painter->drawText(titleRect, Qt::AlignLeft|Qt::AlignVCenter, titleText);

    if (hover || !uniqueTitle) {
        // draw sub-title
        painter->setPen(QPen(option.palette.dark(), 1));
        painter->setFont(subTitleFont);
        painter->drawText(subTitleRect, Qt::AlignLeft|Qt::AlignVCenter, subTitleText);
    }

    painter->restore();

    // draw free space information (for drive icons)
    if (usedSpace >= 0) {
        painter->save();

        QFontMetrics titleMetrics(option.font);
        QFontMetrics subTitleMetrics(subTitleFont);

        qreal actualTextWidth = qMax(titleMetrics.width(titleText), subTitleMetrics.width(subTitleText));

        QSize spaceSize = option.rect.size();
        //kDebug() << "space size is" << spaceSize.rwidth() << "and we're going to lop off"
        //         << (actualTextWidth + decorationRect.width() + ICON_TEXT_MARGIN + 3);
        spaceSize.rwidth() /= 3; //-= (spaceSize.width() * 2.0 / 3.0) /*actualTextWidth +*/ + decorationRect.width() + (ICON_TEXT_MARGIN * 2) + 3;
        spaceSize.rheight() -= 20;

        // check if there is enough space to draw the bar
        qreal textBarGap = (titleRect.width() - actualTextWidth) - (spaceSize.width() + ICON_TEXT_MARGIN);
        //kDebug() << "text bar gap is" << textBarGap;

        if (textBarGap > 0) {
            // if the item view is gradually resized smaller or larger, make the bar fade out/in
            // as enough space for it becomes available
            if (textBarGap < 20.0) {
                painter->setOpacity(textBarGap/20.0);
            }

            QRectF spaceRect = QStyle::alignedRect(option.direction,
                                                   Qt::AlignRight, spaceSize, contentRect);

            // add spacing between item text and free-space bar and tweak the position slightly
            // to give a sharp outline when drawn with anti-aliasing enabled
            spaceRect.translate(0.5, 3.5);

            QColor fillBrush = KColorUtils::mix(Qt::green, Qt::yellow, (usedSpace / (freeSpace+usedSpace)));
            QColor penColor = option.palette.mid().color();
            if (!hover) {
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

QFont ItemDelegate::fontForSubTitle(const QFont& titleFont) const
{
    QFont subTitleFont = titleFont;
    subTitleFont.setPointSize(qMax(subTitleFont.pointSize() - 2,
                              KGlobalSettings::smallestReadableFont().pointSize()));
    return subTitleFont;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    QSize size = option.rect.size();

    QFontMetrics metrics(option.font);

/*    size.rwidth() += ICON_TEXT_MARGIN +
                     qMax(metrics.width(index.data(Qt::DisplayRole).value<QString>()),
                          metrics.width(index.data(SubTitleRole).value<QString>()));*/
    QFont subTitleFont = option.font;
    subTitleFont.setPointSize(qMax(subTitleFont.pointSize() - 2,
                                   KGlobalSettings::smallestReadableFont().pointSize()));
    QFontMetrics subMetrics(subTitleFont);
    size.setHeight(qMax(option.decorationSize.height(), qMax(size.height(), metrics.height() + subMetrics.ascent()) + 3));
//    kDebug() << "size hint is" << size << (metrics.height() + subMetrics.ascent());

    return size;
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

