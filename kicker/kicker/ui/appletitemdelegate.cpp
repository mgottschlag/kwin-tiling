/**
  * This file is part of the KDE project
  * Copyright (C) 2006 Rafael Fernández López <ereslibre@gmail.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "appletitemdelegate.h"

class AppletItemDelegate::Private
{
public:
    QString title(const QModelIndex &index) const;
    QString description(const QModelIndex &index) const;
    QPixmap icon(const QModelIndex &index, int width, int height) const; 
    int calculateCenter(const QRect &rect, int pixmapHeight) const;

    int iconWidth;
    int iconHeight;
    int minimumItemWidth;
    int leftMargin;
    int rightMargin;
    int separatorPixels;
};

QString AppletItemDelegate::Private::title(const QModelIndex &index) const
{
    return index.model()->data(index, Qt::DisplayRole).toString();
}

QString AppletItemDelegate::Private::description(const QModelIndex &index) const
{
    return index.model()->data(index, SecondaryDisplayRole).toString();
}

QPixmap AppletItemDelegate::Private::icon(const QModelIndex &index, int width, int height) const
{
    QVariant icon = index.model()->data(index, Qt::DecorationRole);

    return icon.value<QIcon>().pixmap(width, height);
}

int AppletItemDelegate::Private::calculateCenter(const QRect &rect, int pixmapHeight) const
{
    return (rect.height() / 2) - (pixmapHeight / 2);
}

AppletItemDelegate::AppletItemDelegate(QObject *parent)
    : QItemDelegate(parent)
    , d(new Private)
{
}

AppletItemDelegate::~AppletItemDelegate()
{
    delete d;
}

void AppletItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fontMetrics = painter->fontMetrics();

    QColor unselectedTextColor = option.palette.text().color();
    QColor selectedTextColor = option.palette.highlightedText().color();
    QPen currentPen = painter->pen();
    QPen unselectedPen = QPen(currentPen);
    QPen selectedPen = QPen(currentPen);

    unselectedPen.setColor(unselectedTextColor);
    selectedPen.setColor(selectedTextColor);

    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(selectedPen);
    }
    else
    {
        painter->setPen(unselectedPen);
    }

    QPixmap iconPixmap = d->icon(index, 48, 48);

    QFont title(painter->font());
    QFont previousFont(painter->font());

    title.setPointSize(title.pointSize() + 2);
    title.setWeight(QFont::Bold);

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setFont(title);

    QString display = painter->fontMetrics().elidedText(d->title(index), Qt::ElideRight, option.rect.width() - d->leftMargin - d->rightMargin - iconPixmap.width() - d->separatorPixels);

    QString secondaryDisplay = fontMetrics.elidedText(d->description(index), Qt::ElideRight, option.rect.width() - d->leftMargin - d->rightMargin - iconPixmap.width() - d->separatorPixels);

    painter->drawText(d->leftMargin + d->separatorPixels + iconPixmap.width(), d->separatorPixels + option.rect.top(), painter->fontMetrics().width(display), painter->fontMetrics().height(), Qt::AlignLeft, display);

    painter->setFont(previousFont);

    painter->drawText(d->leftMargin + d->separatorPixels + iconPixmap.width(), option.rect.height() - d->separatorPixels - fontMetrics.height() + option.rect.top(), painter->fontMetrics().width(secondaryDisplay), painter->fontMetrics().height(), Qt::AlignLeft, secondaryDisplay);

    painter->drawPixmap(d->leftMargin, d->calculateCenter(option.rect, iconPixmap.height()) + option.rect.top(), iconPixmap);

    painter->restore();

}

QSize AppletItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int itemWidth = qMax(option.rect.width(), d->minimumItemWidth);
    int itemHeight = (d->separatorPixels * 2) + d->iconHeight;

    return QSize(itemWidth, itemHeight);
}

void AppletItemDelegate::setIconSize(int width, int height)
{
    d->iconWidth = width;
    d->iconHeight = height;
}

void AppletItemDelegate::setMinimumItemWidth(int minimumItemWidth)
{
    d->minimumItemWidth = minimumItemWidth;
}

void AppletItemDelegate::setLeftMargin(int leftMargin)
{
    d->leftMargin = leftMargin;
}

void AppletItemDelegate::setRightMargin(int rightMargin)
{
    d->rightMargin = rightMargin;
}

void AppletItemDelegate::setSeparatorPixels(int separatorPixels)
{
    d->separatorPixels = separatorPixels;
}

#include "appletitemdelegate.moc"
