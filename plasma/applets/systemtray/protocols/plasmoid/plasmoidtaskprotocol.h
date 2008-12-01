/***************************************************************************
 *   plasmoidprotocol.h                                                    *
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

#ifndef PLASMOIDTASKPROTOCOL_H
#define PLASMOIDTASKPROTOCOL_H

#include "../../core/protocol.h"

#include <QHash>

namespace SystemTray
{

class PlasmoidTask;

class PlasmoidProtocol : public Protocol
{
    Q_OBJECT

public:
    PlasmoidProtocol(QObject * parent);
    ~PlasmoidProtocol();

    void init();

private slots:
    void cleanupTask(QString typeId);
    void newTask(QString appletName);

private:
    QHash<QString, PlasmoidTask*> m_tasks;
};

}


#endif
