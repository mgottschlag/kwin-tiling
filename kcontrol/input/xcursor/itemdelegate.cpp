/*
 * Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "itemdelegate.h"
#include "cursortheme.h"

#include <QModelIndex>
#include <QPainter>

namespace
{
    const int decorationMargin = 8;
}


ItemDelegate::ItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}


ItemDelegate::~ItemDelegate()
{
}


QString ItemDelegate::firstLine(const QModelIndex &index) const
{
    if (index.isValid())
        return index.model()->data(index, Qt::DisplayRole).toString();

    return QString();
}


QString ItemDelegate::secondLine(const QModelIndex &index) const
{
    if (index.isValid())
        return index.model()->data(index, CursorTheme::DisplayDetailRole).toString();

	return QString();
}


QPixmap ItemDelegate::decoration(const QModelIndex &index) const
{
    if (index.isValid())
        return qvariant_cast<QPixmap>(index.model()->data(index, Qt::DecorationRole));

    return QPixmap();
}


QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return QSize();

    QFont normalfont = option.font;
    QFont boldfont = normalfont;
    boldfont.setBold(true);

    // Extract the items we want to measure
    QString firstRow   = firstLine(index);
    QString secondRow  = secondLine(index);

    // Compute the height
    QFontMetrics fm1(boldfont);
    QFontMetrics fm2(normalfont);
    int height = fm1.lineSpacing() + fm2.lineSpacing();
    height = qMax(height, option.decorationSize.height());

    // Compute the text width
    int width = fm1.width(firstRow);
    width = qMax(width, fm2.width(secondRow));

    // Add decoration width + margin
    width += option.decorationSize.width() + decorationMargin;

    return QSize(width, height + 16);
}


QPalette::ColorRole ItemDelegate::foregroundRole(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    if (option.state & QStyle::State_Selected && option.showDecorationSelected)
        return QPalette::HighlightedText;

    return QPalette::Text;
}


QPalette::ColorRole ItemDelegate::backgroundRole(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected && option.showDecorationSelected)
        return QPalette::Highlight;

    const QStyleOptionViewItemV2 *option2 = qstyleoption_cast<const QStyleOptionViewItemV2*>(&option);

    if (option2)
        return option2->features & QStyleOptionViewItemV2::Alternate ?
                QPalette::AlternateBase : QPalette::Base;
    else
        return index.row() % 2 ? QPalette::AlternateBase : QPalette::Base;
}


void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();

    QFont normalfont = option.font;
    QFont boldfont = normalfont;
    boldfont.setBold(true);

    QString firstRow  = firstLine(index);
    QString secondRow = secondLine(index);
    QPixmap pixmap    = decoration(index);

    // Figure out the text and background colors
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    QColor textcol = option.palette.color(cg, foregroundRole(option, index));
    QBrush bgbrush = option.palette.brush(cg, backgroundRole(option, index));

    // Draw the background
    painter->fillRect(option.rect, bgbrush);

    // Draw the icon
    int x = option.rect.left() + (option.decorationSize.width() - pixmap.width() + decorationMargin) / 2;
    int y = option.rect.top() + (option.rect.height() - pixmap.height()) / 2;

    QRect pixmapRect = QStyle::visualRect(option.direction, option.rect,
                                          QRect(x, y, pixmap.width(), pixmap.height()));

    painter->drawPixmap(pixmapRect.x(), pixmapRect.y(), pixmap);

    // Draw the text
    QFontMetrics fm1(boldfont);
    QFontMetrics fm2(normalfont);

    int textAreaHeight = fm1.lineSpacing() + fm2.lineSpacing();

    x = option.rect.left() + option.decorationSize.width() + decorationMargin;
    int y1 = option.rect.top() + (option.rect.height() - textAreaHeight) / 2;
    int y2 = y1 + fm1.lineSpacing();

    QRect firstRowRect = QStyle::visualRect(option.direction, option.rect,
                                            QRect(x, y1, fm1.width(firstRow), fm1.lineSpacing()));

    QRect secondRowRect = QStyle::visualRect(option.direction, option.rect,
                                             QRect(x, y2, fm2.width(secondRow), fm2.lineSpacing()));

    painter->setPen(textcol);

    // First line
    painter->setFont(boldfont);
    painter->drawText(firstRowRect, Qt::AlignCenter, firstRow);

    // Second line
    painter->setFont(normalfont);
    painter->drawText(secondRowRect, Qt::AlignCenter, secondRow);

    painter->restore();
}

