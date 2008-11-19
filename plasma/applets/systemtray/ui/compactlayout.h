/***************************************************************************
 *   compactlayout.h                                                       *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef COMPACTLAYOUT_H
#define COMPACTLAYOUT_H

#include <QGraphicsLayout>


namespace SystemTray
{

class CompactLayout : public QGraphicsLayout
{
public:
    CompactLayout(QGraphicsLayoutItem *parent = 0);
    ~CompactLayout();

    qreal spacing() const;
    void setSpacing(qreal spacing);

    void insertItem(int index, QGraphicsLayoutItem *item);
    void addItem(QGraphicsLayoutItem *item);
    void removeItem(QGraphicsLayoutItem *item);
    bool containsItem(QGraphicsLayoutItem *item) const;

    int count() const;
    void setGeometry(const QRectF &rect);
    QGraphicsLayoutItem* itemAt(int i) const;
    void removeAt(int index);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

private:
    class Private;
    Private* const d;
};

}


#endif
