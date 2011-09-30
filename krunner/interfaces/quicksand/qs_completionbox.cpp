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

#include <QKeyEvent>
#include <QPoint>
#include <QResizeEvent>

#include <KDebug>

#include "qs_completionbox.h"
#include "qs_statusbar.h"

namespace QuickSand {

QsCompletionBox::QsCompletionBox(QWidget *parent)
    : KCompletionBox(parent),
        m_offset(0,0)
{
    m_status = new QsStatusBar(this);
    connect(model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(slotRowsChanged(QModelIndex,int,int)));
    connect(model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotRowsChanged(QModelIndex,int,int)));
    connect(this, SIGNAL(currentRowChanged(int)), m_status, SLOT(slotCurrentRowChanged(int)));
}

QRect QsCompletionBox::calculateGeometry() const
{
    QRect geom = KCompletionBox::calculateGeometry();
    geom.setHeight(geom.height() + m_status->geometry().height());
    geom.setWidth(geom.width() * 3/4);
    return geom;
}

QSize QsCompletionBox::minimumSizeHint() const
{
    return sizeHint();
}

QSize QsCompletionBox::sizeHint() const
{
    return calculateGeometry().size();
}

void QsCompletionBox::popup()
{
    KCompletionBox::popup();
    resize(calculateGeometry().size());
}

void QsCompletionBox::updateGeometries()
{
    KCompletionBox::updateGeometries();
    int statusHeight = m_status->geometry().height();

//         setGeometry(calculateGeometry());

    setViewportMargins(0,0,0,statusHeight);

    QRect vg = viewport()->geometry();
    int statusTop = vg.bottom();

    m_status->setGeometry(vg.left(), statusTop, vg.width(), statusHeight);
}

void QsCompletionBox::slotRowsChanged(const QModelIndex &, int, int)
{
    int rows = model()->rowCount();
    m_status->setTotalRows(rows);
}

QPoint QsCompletionBox::globalPositionHint() const
{
    QWidget *p = qobject_cast<QWidget*>(parent());
    if (!p) {
        return QPoint();
    }
    return p->mapToGlobal(QPoint(p->width(), 22));
}

} // Namespace QuickSand

#include "qs_completionbox.moc"
