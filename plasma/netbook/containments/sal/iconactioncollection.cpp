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

#include <QAction>

#include "iconactioncollection.h"


IconActionCollection::IconActionCollection(Plasma::Applet *applet, QObject *parent)
      : QObject(parent),
        m_immutability(Plasma::Mutable)
{
    if (applet) {
        connect (applet, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
        m_immutability = Plasma::Mutable;
    }
}

IconActionCollection::~IconActionCollection()
{ 
}

void IconActionCollection::addAction(QAction *action)
{
    if (action) {
        m_actions.insert(action);
        connect (action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));
        action->setVisible(m_immutability == Plasma::Mutable);
        action->setEnabled(m_immutability == Plasma::Mutable);
    }
}

void IconActionCollection::removeAction(QAction *action)
{
    m_actions.remove(action);
}



QList<QAction *> IconActionCollection::actions() const
{
    return m_actions.toList();
}



void IconActionCollection::actionDestroyed(QObject *object)
{
    m_actions.remove(static_cast<QAction *>(object));
}

void IconActionCollection::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    m_immutability = immutability;

    foreach (QAction *action, m_actions) {
        action->setVisible(immutability == Plasma::Mutable);
        action->setEnabled(immutability == Plasma::Mutable);
    }
}


#include "iconactioncollection.moc"
