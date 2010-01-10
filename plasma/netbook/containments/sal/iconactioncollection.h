/*
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_ICONACTIONCOLLECTION_H
#define PLASMA_ICONACTIONCOLLECTION_H

#include <QObject>

#include <Plasma/Applet>
#include <Plasma/Plasma>

class IconActionCollection : public QObject
{
    Q_OBJECT

public:
    IconActionCollection(Plasma::Applet *applet, QObject *parent = 0);
    ~IconActionCollection();

    void addAction(QAction *action);
    void removeAction(QAction *action);

    QList<QAction *> actions() const;

protected Q_SLOTS:
    void actionDestroyed(QObject *object);
    void immutabilityChanged(Plasma::ImmutabilityType immutability);

private:
    QSet<QAction *> m_actions;
    Plasma::ImmutabilityType m_immutability;
};

#endif
