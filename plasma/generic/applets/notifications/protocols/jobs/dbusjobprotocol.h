/***************************************************************************
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
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

#ifndef DBUSJOBPROTOCOL_H
#define DBUSJOBPROTOCOL_H

#include "../../core/protocol.h"
#include "../../core/notificationsmanager.h"

#include <plasma/dataengine.h>


class DBusJob;

class DBusJobProtocol : public Protocol
{
    Q_OBJECT

public:
    DBusJobProtocol(Manager *parent);
    ~DBusJobProtocol();
    void init();

private slots:
    void prepareJob(const QString &source);
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void removeJob(const QString &source);
    //void relayAction(const QString &source, const QString &actionName);
    void suspend(const QString &source);
    void resume(const QString &source);
    void stop(const QString &source);

private:
    Manager *m_manager;
    Plasma::DataEngine *m_engine;
    QHash<QString, DBusJob*> m_jobs;
};



#endif
