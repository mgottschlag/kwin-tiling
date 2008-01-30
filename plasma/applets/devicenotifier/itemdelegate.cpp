/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>
    Copyright 2007 Alexis Menard <darktears31@gmail.com>


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

#include "itemdelegate.h"

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

using namespace Notifier;

ItemDelegate::ItemDelegate()
{
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool hover = option.state & (QStyle::State_Selected|QStyle::State_MouseOver|QStyle::State_HasFocus);
    QRect contentRect = option.rect;

    QRect decorationRect = QStyle::alignedRect(option.direction,
                                               option.decorationPosition == QStyleOptionViewItem::Left ? Qt::AlignLeft : Qt::AlignRight,
                                               option.decorationSize,
                                               contentRect);
    QSize textSize(option.rect.width() - decorationRect.width() - ICON_TEXT_MARGIN,
                   option.rect.height() - 2);

    Qt::Alignment textAlignment = option.decorationAlignment & Qt::AlignRight ? Qt::AlignLeft : Qt::AlignRight;

    QRect textRect = QStyle::alignedRect(option.direction, textAlignment, textSize, contentRect.adjusted(0, 2, 0, 0));
    QString titleText = index.data(Qt::DisplayRole).value<QString>();
    QString subTitleText = index.data(ActionRole).value<QString>();

    if (subTitleText.isEmpty()) {
        subTitleText = " ";
    }

    QFont subTitleFont = fontForSubTitle(option.font);

    QFont titleFont(option.font);

    QFontMetrics titleMetrics(titleFont);
    QFontMetrics subTitleMetrics(subTitleFont);
    QRect textAreaRect = contentRect;
    qreal actualTextWidth = qMax(titleMetrics.width(titleText), subTitleMetrics.width(subTitleText));
    textAreaRect.adjust(decorationRect.width() + ICON_TEXT_MARGIN - 3,
                        0,
                        (int)(-(textRect.width() - actualTextWidth) + 3),
                        1);

    if (hover) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor backgroundColor = option.palette.color(QPalette::Highlight);
        // use a slightly translucent version of the palette's highlight color
        // for the background
        backgroundColor.setAlphaF(0.5);
        painter->setBrush(QBrush(backgroundColor));
        painter->drawPath(roundedRectangle(textAreaRect, 5));
        painter->restore();
    }

    // draw icon
    QIcon decorationIcon = index.data(Qt::DecorationRole).value<QIcon>();

    if (!hover) {
        painter->save();
        painter->setOpacity(0.7);
    }

    decorationIcon.paint(painter, decorationRect, option.decorationAlignment);

    if (!hover) {
        painter->restore();
    }

    painter->save();

    // draw title
    painter->setFont(titleFont);

    textAreaRect.setHeight(textAreaRect.height()/2);

    painter->drawText(textAreaRect, Qt::AlignLeft|Qt::AlignVCenter, titleText);

    // draw sub-title
    painter->setFont(subTitleFont);

    if (!hover) {
        painter->setPen(QPen(Qt::gray));
    }
    textAreaRect.translate(0, textAreaRect.height());

    painter->drawText(textAreaRect, Qt::AlignLeft|Qt::AlignVCenter, subTitleText);

    painter->restore();

}

QFont ItemDelegate::fontForSubTitle(const QFont &titleFont) const
{
    QFont subTitleFont = titleFont;
    subTitleFont.setPointSize(qMax(subTitleFont.pointSize() - 2,
                              KGlobalSettings::smallestReadableFont().pointSize()));
    return subTitleFont;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    QFontMetrics metrics(option.font);

    QFont subTitleFont = option.font;
    subTitleFont.setPointSize(qMax(subTitleFont.pointSize() - 2,
                                   KGlobalSettings::smallestReadableFont().pointSize()));
    QFontMetrics subMetrics(subTitleFont);

    int height = qMax(option.decorationSize.height(), metrics.height() + subMetrics.ascent() + 3);

    QString titleText = index.data(Qt::DisplayRole).value<QString>();
    QString subTitleText = index.data(ActionRole).value<QString>();

    int width = qMax(metrics.width(titleText), subMetrics.width(subTitleText));
    width += option.decorationSize.width() + ICON_TEXT_MARGIN;

    return QSize(width, height);
}

// Taken from kdelibs/kio/kio/kfileitemdelegate.cpp
QPainterPath ItemDelegate::roundedRectangle(const QRectF &rect, qreal radius) const
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
