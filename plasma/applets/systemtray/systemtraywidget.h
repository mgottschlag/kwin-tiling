/***************************************************************************
 *   systemtraywidget.h                                                    *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin <rodin.alexander@gmail.com>        *
 *   Copyright (C) 2007 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef SYSTRAYWIDGET_H
#define SYSTRAYWIDGET_H

// Qt
#include <QGridLayout>
#include <QWidget>

// Xlib
#include <X11/Xdefs.h>

class SystemTrayWidget: public QWidget
{
Q_OBJECT

public:
    SystemTrayWidget(QWidget *parent);

    void setOrientation(Qt::Orientation);

    static const int MARGIN = 4;

protected:
    bool x11Event(XEvent *event);

Q_SIGNALS:
    void sizeShouldChange();

private slots:
    void relayoutContainers(QObject *removeContainer = 0);

private:
    void addWidgetToLayout(QWidget *widget);
    void init();

    QGridLayout *m_mainLayout;
    Qt::Orientation m_orientation;
    int m_nextRow;
    int m_nextColumn;

    // These need to remain allocated for the duration of our lifetime
    Atom m_selectionAtom;
    Atom m_opcodeAtom;
};

#endif // SYSTRAYWIDGET_H
