/*
 *   Copyright (C) 2007-2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#include <QStyle>
#include <QStyleOptionHeader>
#include <QStylePainter>

#include <KGlobalSettings>
#include <KDebug>
#include <KLocale>

#include "qs_statusbar.h"

namespace QuickSand {

QsStatusBar::QsStatusBar(QWidget *parent)
    : QLabel(parent),
    m_currentItem(0),
    m_totalItems(0)
{
}

// Patterned after QHeaderView
void QsStatusBar::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);

    QRect rect(0, 0, geometry().width(), geometry().height());

    QStyleOptionHeader opt;
    opt.initFrom(this);
    opt.state = QStyle::State_None | QStyle::State_Raised;
    opt.state |= QStyle::State_Horizontal;
    opt.state |= QStyle::State_Enabled;

    opt.rect = rect;
    opt.section = 0;
    opt.textAlignment = Qt::AlignRight;

    opt.iconAlignment = Qt::AlignVCenter;
    opt.text = i18nc("%1 current item number, %2 total number of items","%1 of %2", m_currentItem, m_totalItems);


    opt.position = QStyleOptionHeader::OnlyOneSection;
    opt.orientation = Qt::Horizontal;

    QFont font = painter.font();
    font.setPointSize(qMax(font.pointSize() - 2,
                              KGlobalSettings::smallestReadableFont().pointSize()));
    painter.setFont(font);

    painter.drawControl(QStyle::CE_Header, opt);
}

void QsStatusBar::setTotalRows(int total)
{
    m_totalItems = total;
}

void QsStatusBar::slotCurrentRowChanged(int newRow)
{
    m_currentItem = newRow + 1;
    update();
}

} //namespace QuickSand

#include "qs_statusbar.moc"
