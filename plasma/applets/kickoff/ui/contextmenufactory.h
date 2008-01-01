/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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
#ifndef CONTEXTMENUFACTORY_H
#define CONTEXTMENUFACTORY_H

// Qt
#include <QAction>

// Plasma
#include <plasma/applet.h>

class QAbstractItemView;

namespace Kickoff
{

class ContextMenuFactory : public QObject
{
Q_OBJECT 

public:
    ContextMenuFactory(QObject *parent = 0);
    ~ContextMenuFactory();

    void setViewActions(QAbstractItemView *view,const QList<QAction*>& actions);
    QList<QAction*> viewActions(QAbstractItemView *view) const;

    void setApplet(Plasma::Applet *applet);

public Q_SLOTS:
    void showContextMenu(QAbstractItemView *view,const QPoint& pos); 

private:
    class Private;
    Private * const d;
};

}

#endif // CONTEXTMENUFACTORY_H 
