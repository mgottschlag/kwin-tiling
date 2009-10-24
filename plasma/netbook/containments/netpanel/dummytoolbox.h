/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DUMMYTOOLBOX_H
#define DUMMYTOOLBOX_H


#include <Plasma/AbstractToolBox>


class DummyToolBox : public Plasma::AbstractToolBox
{
    Q_OBJECT
    Q_PROPERTY(bool showing READ isShowing WRITE setShowing )
public:
    DummyToolBox(Plasma::Containment *parent = 0);
    ~DummyToolBox();

    bool isShowing() const;
    void setShowing(const bool show);

    void addTool(QAction *action);
    void removeTool(QAction *action);

private:
    bool m_showing;
};

#endif
