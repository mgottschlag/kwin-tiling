/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "core/itemhandlers.h"

// Qt
#include <QUrl>
#include <QtDebug>

// KDE
#include <KService>
#include <KToolInvocation>
#include <solid/powermanagement.h>

// KDE Base
#include <libworkspace/kworkspace.h>

// Local
#include "core/recentapplications.h"

using namespace Kickoff;

bool ServiceItemHandler::openUrl(const QUrl& url) const
{
    int result = KToolInvocation::startServiceByDesktopPath(url.toString(),QStringList(),0,0,0,"",true);

    if (result == 0) {
        KService::Ptr service = KService::serviceByDesktopPath(url.toString());

        if (!service.isNull()) {
            RecentApplications::self()->add(service);
        } else {
            qWarning() << "Failed to find service for" << url;
            return false;
        }
    }

    return result == 0;
}
bool LeaveItemHandler::openUrl(const QUrl& url) const
{
    QString action = url.path().remove('/');

    KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
    KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeNone;

    if (action == "logout") {
        type = KWorkSpace::ShutdownTypeNone;
    } else if (action == "lock") {
        qDebug() << "Locking screen"; 
    } else if (action == "switch") {
        qDebug() << "Switching user";
    } else if (action == "restart") {
        type = KWorkSpace::ShutdownTypeReboot;
    } else if (action == "shutdown") {
        type = KWorkSpace::ShutdownTypeHalt;
    } else if (action == "sleep") {
        Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState,0,0);
        return true;
    } else if (action == "hibernate") {
        Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState,0,0);
        return true;
    } else {
        return false;
    }

    return KWorkSpace::requestShutDown(confirm,type);
}
