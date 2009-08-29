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

#ifndef WIDGET
#define WIDGET

#include <QObject>
#include <QPointer>

namespace Plasma
{
    class Applet;
} // namespace Plasma

class Widget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(int id READ id)


public:
    Widget(Plasma::Applet *applet, QObject *parent = 0);
    ~Widget();

    uint id() const;
    QString type() const;

public Q_SLOTS:
    void remove();

private:
    QPointer<Plasma::Applet> m_applet;
};

#endif

