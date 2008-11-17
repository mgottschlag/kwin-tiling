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

#include "dbusjob.h"

#include <KDebug>


namespace SystemTray
{
namespace DBus
{


class Job::Private
{
public:
    QString source;
};


Job::Job(const QString &source, QObject *parent)
    : SystemTray::Job(parent),
      d(new Private())
{
    d->source = source;
}


Job::~Job()
{
    emit jobDeleted(d->source);
    delete d;
}


/**
void Notification::triggerAction(const QString &actionId)
{
    emit actionTriggered(d->source, actionId);
}
*/

void Job::suspend()
{
    emit suspend(d->source);
    kDebug() << "suspend";

}

void Job::resume()
{
    emit resume(d->source);
    kDebug() << "resume";
}

void Job::stop()
{
    emit stop(d->source);
    kDebug() << "cancel";
}
//void Notification::


}
}
