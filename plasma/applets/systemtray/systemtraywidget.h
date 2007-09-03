/***************************************************************************
 *   systemtraywidget.h                                                    *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin                                    *
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

#ifndef QSYSTRAY_H
#define QSYSTRAY_H

// Qt
#include <QWidget>
#include <QHash>

// Xlib
#include <X11/Xdefs.h>

class QHBoxLayout;
class QX11EmbedContainer;

class SystemTrayWidget: public QWidget
{
Q_OBJECT

public:
    SystemTrayWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

protected:
    bool x11Event(XEvent *event);
    bool event(QEvent *event);

Q_SIGNALS:
    void sizeChanged();

private slots:
    void init();
    void embedWindow(WId id);
    void discardWindow(WId id);

private:
    typedef QHash<WId, QX11EmbedContainer*> ContainersList;
    
    ContainersList m_containers;
    QHBoxLayout *m_layout;
    Atom m_selectionAtom;
    Atom m_opcodeAtom;
};

#endif // QSYSTRAY_H
