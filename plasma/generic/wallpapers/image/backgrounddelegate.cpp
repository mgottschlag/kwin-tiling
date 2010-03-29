/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2010 Dario Andres Rodriguez  <andresbajotierra@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgrounddelegate.h"

#include <QApplication>
#include <QTextDocument>
#include <QPainter>

#include <KDebug>
#include <KGlobalSettings>
#include <KLocalizedString>

BackgroundDelegate::BackgroundDelegate(QObject *listener, float ratio, QObject *parent)
    : QAbstractItemDelegate(parent), 
      m_listener(listener),
      m_ratio(ratio)
{
    m_maxHeight = SCREENSHOT_SIZE;
    m_maxWidth = int(m_maxHeight * m_ratio);
}

void BackgroundDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const QString author = index.model()->data(index, AuthorRole).toString();
    const QString resolution = index.model()->data(index, ResolutionRole).toString();
    const QPixmap pix = index.model()->data(index, ScreenshotRole).value<QPixmap>();

    // Highlight selected item
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    // Draw wallpaper thumbnail
    if (!pix.isNull()) {
        const int x = (option.rect.width() - pix.width()) / 2;
        // the y ix aligned to the baseline of the icons
        const int y = MARGIN + qMax(0, m_maxHeight - pix.height());
        QRect imgRect = QRect(option.rect.topLeft(), pix.size()).translated(x, y);
        painter->drawPixmap(imgRect, pix);
    }

    //Use a QTextDocument to layout the text
    QTextDocument document;

    QString html = QString("<strong>%1</strong>").arg(title);

    if (!author.isEmpty()) {
        QString authorCaption = i18nc("Caption to wallpaper preview, %1 author name",
                              "by %1", author);

        html += QString("<br /><span style=\"font-size: %1pt;\">%2</span>")
                .arg(KGlobalSettings::smallestReadableFont().pointSize())
                .arg(authorCaption);
    }

    if (!resolution.isEmpty()) {
        html += QString("<br /><span style=\"font-size: %1pt;\">%2</span>")
                .arg(KGlobalSettings::smallestReadableFont().pointSize())
                .arg(resolution);
    }

    //Set the text color according to the item state
    QColor color;
    if (option.state & QStyle::State_Selected) {
        color = QApplication::palette().brush(QPalette::HighlightedText).color();
    }else{
        color = QApplication::palette().brush(QPalette::Text).color();
    }
    html = QString("<div style=\"color: %1\" align=\"center\">%2</div>").arg(color.name()).arg(html);

    document.setHtml(html);

    //Calculate positioning
    int x = option.rect.left();// + MARGIN * 2 + m_maxWidth;

    //Enable word-wrap
    document.setTextWidth(option.rect.width());

    //Center text on the row
    int y = option.rect.top() + m_maxHeight + MARGIN * 2; //qMax(0 ,(int)((option.rect.height() - document.size().height()) / 2));

    //Draw text
    painter->save();
    painter->translate(x, y);
    document.drawContents(painter, QRect(QPoint(0, 0), option.rect.size()));
    painter->restore();
}

QSize BackgroundDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const QString author = index.model()->data(index, AuthorRole).toString();
    const int fontSize = KGlobalSettings::smallestReadableFont().pointSize();

    //Generate a sample complete entry (with the real title) to calculate sizes
    QTextDocument document;
    QString html = QString("<strong>%1</strong><br />").arg(title);
    if (!author.isEmpty()) {
        html += QString("<span style=\"font-size: %1pt;\">by %2</span><br />").arg(fontSize).arg(author);
    }
    html += QString("<span style=\"font-size: %1pt;\">1600x1200</span>").arg(fontSize);

    document.setHtml(html);
    document.setTextWidth(m_maxWidth + MARGIN * 2);

    return QSize(qMax(m_maxWidth + MARGIN * 2, (int)(document.size().width())),
                 m_maxHeight + MARGIN * 2 + (int)(document.size().height()));
}

