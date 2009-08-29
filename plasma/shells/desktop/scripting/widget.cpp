/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "widget.h"

#include <Plasma/Applet>

Widget::Widget(Plasma::Applet *applet, QObject *parent)
    : QObject(parent),
      m_applet(applet)
{

}

Widget::~Widget()
{

}

uint Widget::id() const
{
    if (m_applet) {
        return m_applet->id();
    }

    return 0;
}

QString Widget::type() const
{
    if (m_applet) {
        return m_applet->pluginName();
    }

    return QString();
}

void Widget::remove()
{
    if (m_applet) {
        m_applet->destroy();
        m_applet = 0;
    }
}


#include "widget.moc"

