/*  This file is part of the KDE project
    Copyright (C) 2008 Alessandro Diaferia <alediaferia@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
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
#include "pkitemdelegate.h"
#include "PoliciesModel.h"

#include <QStyleOptionViewItemV4>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>

#include <KIcon>
#include <KGlobalSettings>
#include <KDebug>

const int ITEM_ROW_HEIGHT = 32;
const int GROUP_ROW_HEIGHT = 22;
const int ICON_SIZE = 22;
const int MARGIN = 1;

PkItemDelegate::PkItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{}

PkItemDelegate::~PkItemDelegate()
{}

void PkItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt(option);

    const int ROW_HEIGHT = index.data(PolkitKde::PoliciesModel::IsGroupRole).toBool() ? GROUP_ROW_HEIGHT : ITEM_ROW_HEIGHT;

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QPixmap pixmap(opt.rect.size());
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-opt.rect.topLeft());

    QRect clipRect(opt.rect);
    p.setClipRect(clipRect);

    // here we draw the icon
    KIcon icon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));

    QIcon::Mode iconMode = QIcon::Normal;

    if (opt.state & QStyle::State_MouseOver) {
        iconMode = QIcon::Active;
    }

    QRect iconRect(opt.rect.topLeft(), QSize(ICON_SIZE, ICON_SIZE));
    const QRect iconPlaceRect(opt.rect.topLeft(), QSize(ROW_HEIGHT, ROW_HEIGHT));
    iconRect.moveCenter(iconPlaceRect.center());
    icon.paint(&p, iconRect, Qt::AlignCenter, iconMode);

    QColor foregroundColor = (opt.state.testFlag(QStyle::State_Selected)) ?
                             opt.palette.color(QPalette::HighlightedText) : opt.palette.color(QPalette::Text);

    p.setPen(foregroundColor);

    // here we draw the action description
    clipRect.setSize(QSize(opt.rect.width() - ROW_HEIGHT - MARGIN, ROW_HEIGHT / 2));
    clipRect.translate(ROW_HEIGHT + MARGIN, 0);
    p.setClipRect(clipRect);

    QFont descriptionFont = opt.font;
    if (index.model()->hasChildren(index)) {
        descriptionFont.setBold(true);
    }
    descriptionFont.setPointSize(descriptionFont.pointSize());
    p.setFont(descriptionFont);

    // let's differ groups from items
    if (index.data(PolkitKde::PoliciesModel::IsGroupRole).toBool()) {
        clipRect.setSize(QSize(clipRect.width(), GROUP_ROW_HEIGHT));
        p.setClipRect(clipRect);
        p.drawText(clipRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    } else {
        p.drawText(clipRect, Qt::AlignLeft | Qt::AlignBottom, index.data(Qt::DisplayRole).toString());

        // here we draw the action raw name
        clipRect.translate(0, ITEM_ROW_HEIGHT / 2);
        p.setClipRect(clipRect);

        QFont actionNameFont = KGlobalSettings::smallestReadableFont();
        actionNameFont.setItalic(true);
        p.setFont(actionNameFont);
        p.drawText(clipRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(PolkitKde::PoliciesModel::PathRole).toString());
    }

    p.end();

    painter->drawPixmap(opt.rect.topLeft(), pixmap);
}

QSize PkItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont dFont = option.font;


    const QFont rFont = KGlobalSettings::smallestReadableFont();

    QFontMetrics d_fm(dFont); // description font
    QFontMetrics r_fm(rFont); // raw string font

//     PolkitKde::AuthorizationsModel::EntryType type = (PolkitKde::AuthorizationsModel::EntryType)index.data(PolkitKde::AuthorizationsModel::Entry48Role).toInt();
    if (index.data(PolkitKde::PoliciesModel::IsGroupRole).toBool()) {
        dFont.setBold(true);
        d_fm = QFontMetrics(dFont);
        return QSize(qMax(d_fm.width(index.data(Qt::DisplayRole).toString()),
                          d_fm.width(index.data(PolkitKde::PoliciesModel::PathRole).toString())),
                     qMax(GROUP_ROW_HEIGHT, d_fm.height()));
    }

    return QSize(qMax(d_fm.width(index.data(Qt::DisplayRole).toString()),
                      d_fm.width(index.data(PolkitKde::PoliciesModel::PathRole).toString())),
                 qMax(ITEM_ROW_HEIGHT, d_fm.height() + r_fm.height()));
}
