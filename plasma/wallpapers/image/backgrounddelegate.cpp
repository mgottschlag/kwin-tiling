/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgrounddelegate.h"

#include <KGlobalSettings>
#include <QPen>
#include <QPainter>
#include "backgroundpackage.h"

BackgroundDelegate::BackgroundDelegate(QObject *listener,
                                       float ratio, QObject *parent)
: QAbstractItemDelegate(parent)
, m_listener(listener)
, m_ratio(ratio)
{
}

void BackgroundDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString author = index.model()->data(index, AuthorRole).toString();
    QPixmap pix = index.model()->data(index, ScreenshotRole).value<QPixmap>();

    // draw selection outline
    if (option.state & QStyle::State_Selected) {
        QPen oldPen = painter->pen();
        painter->setPen(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect.adjusted(2, 2, -2, -2));
        painter->setPen(oldPen);
    }

    // draw pixmap
    int maxheight = Background::SCREENSHOT_HEIGHT;
    int maxwidth = int(maxheight * m_ratio);
    if (!pix.isNull()) {
        QSize sz = pix.size();
        int x = MARGIN + (maxwidth - pix.width()) / 2;
        int y = MARGIN + (maxheight - pix.height()) / 2;
        QRect imgRect = QRect(option.rect.topLeft(), pix.size()).translated(x, y);
        painter->drawPixmap(imgRect, pix);
    }

    // draw text
    painter->save();
    QFont font = painter->font();
    font.setWeight(QFont::Bold);
    painter->setFont(font);
    int x = option.rect.left() + MARGIN * 5 + maxwidth;

    QRect textRect(x,
                   option.rect.top() + MARGIN,
                   option.rect.width() - x - MARGIN * 2,
                   maxheight);
    QString text = title;
    QString authorCaption;
    if (!author.isEmpty()) {
        authorCaption = i18nc("Caption to wallpaper preview, %1 author name",
                              "by %1", author);
        text += '\n' + authorCaption;
    }
    QRect boundingRect = painter->boundingRect(
        textRect, Qt::AlignVCenter | Qt::TextWordWrap, text);
    painter->drawText(boundingRect, Qt::TextWordWrap, title);
    if (!author.isEmpty()) {
        QRect titleRect = painter->boundingRect(boundingRect, Qt::TextWordWrap, title);
        QRect authorRect(titleRect.bottomLeft(), textRect.size());
        painter->setFont(KGlobalSettings::smallestReadableFont());
        painter->drawText(authorRect, Qt::TextWordWrap, authorCaption);
    }

    painter->restore();
}

QSize BackgroundDelegate::sizeHint(const QStyleOptionViewItem &,
                                   const QModelIndex &) const
{
    return QSize(100, Background::SCREENSHOT_HEIGHT + MARGIN * 2);
}

