/***********************************************************************************
* Window List: Plasmoid to show list of opened windows.
* Copyright (C) 2009 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
***********************************************************************************/

#ifndef WINDOWLIST_HEADER
#define WINDOWLIST_HEADER

#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <Plasma/Applet>

#include <kworkspace/kwindowlistmenu.h>

class WindowList : public Plasma::Applet
{
    Q_OBJECT

public:
    WindowList(QObject *parent, const QVariantList &args);
    ~WindowList();

    void init();

public slots:
    void showMenu(bool onlyCurrentDesktop = false);
    void triggered(QAction *action);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private:
    KWindowListMenu *m_listMenu;
    QPoint m_dragStartPosition;
};

#endif
