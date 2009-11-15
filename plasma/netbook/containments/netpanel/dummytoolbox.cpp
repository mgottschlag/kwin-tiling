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

#include "dummytoolbox.h"



DummyToolBox::DummyToolBox(Plasma::Containment *parent)
   : Plasma::AbstractToolBox(parent),
     m_showing(false)
{
}

DummyToolBox::~DummyToolBox()
{
}

bool DummyToolBox::isShowing() const
{
    return m_showing;
}

void DummyToolBox::setShowing(const bool show)
{
    if (show != m_showing) {
        emit toggled();
        emit visibilityChanged(show);
    }

    m_showing = show;
}


void DummyToolBox::addTool(QAction *)
{
    //not supported
}

void DummyToolBox::removeTool(QAction *)
{
    //not supported
}

#include "dummytoolbox.moc"
