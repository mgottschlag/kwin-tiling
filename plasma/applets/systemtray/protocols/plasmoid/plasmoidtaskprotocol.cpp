/***************************************************************************
 *   plasmoidprotocol.cpp                                                  *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
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

#include "plasmoidtask.h"
#include "plasmoidtaskprotocol.h"

#include <KDebug>

namespace SystemTray
{

PlasmoidProtocol::PlasmoidProtocol(QObject *parent)
    : Protocol(parent)
{
}


PlasmoidProtocol::~PlasmoidProtocol()
{
}


void PlasmoidProtocol::init()
{
    // TODO: Load plasmoids from config
    //newTask("battery");
    //newTask("notify");
    //newTask("kuiserver");
    //newTask("mid_control");
}


void PlasmoidProtocol::newTask(QString appletName)
{
    if (m_tasks.contains(appletName)) {
        kDebug() << "Task " << appletName << "is already in here.";
        return;
    }

    kDebug() << "Registering task with the manager" << appletName;
    PlasmoidTask *task = new PlasmoidTask(appletName);

    if (!task->isValid()) {
        // we failed to load our applet *sob*
        delete task;
        return;
    }

    m_tasks[appletName] = task;
    connect(task, SIGNAL(taskDeleted(QString)), this, SLOT(cleanupTask(QString)));
    emit taskCreated(task);
}


void PlasmoidProtocol::cleanupTask(QString typeId)
{
    kDebug() << "task with typeId" << typeId << "removed";
    m_tasks.remove(typeId);
}

}

#include "plasmoidtaskprotocol.moc"
