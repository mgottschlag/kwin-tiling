/*  This file is part of the KDE project
    Copyright (C) 2009 Pino Toscano <pino@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#ifndef MYQLISTWIDGET_H
#define MYQLISTWIDGET_H

#include <QListWidget>

/*
 MyQListWidget is a small hack to avoid QListWidget having a minimum height
 of ~180px, even if the minimumSize is less and the size policy is for having
 a minimum size.
 Tested with Qt 4.5.1 -- worth a Qt bug?
 */
class MyQListWidget : public QListWidget
{
public:
    explicit MyQListWidget(QWidget *parent = 0)
        : QListWidget(parent)
    {}

    virtual QSize sizeHint() const
    {
        QSize s = QListWidget::sizeHint();
        const QSize ms = minimumSizeHint();
        s.setHeight(qMin(s.height(), ms.height()));
        return s;
    }
};

#endif
