/*
 *   Copyright © 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "notificationaction.h"
#include "notificationsengine.h"

#include <kdebug.h>

void NotificationAction::start()
{
    kDebug() << "Trying to perform the action " << operationName() << " on " << destination();
    kDebug() << "actionId: " << parameters()["actionId"].toString();

    if (!m_engine) {
        setErrorText(i18n("The notification dataEngine is not set."));
        setError(-1);
        emitResult();
        return;
    }

    if (operationName() == "invokeAction") {
        const QStringList dest = destination().split(" ");

        if (dest.count() >  1 && !dest[1].toInt()) {
            setErrorText(i18n("Invalid destination: ", destination()));
            setError(-2);
            emitResult();
            return;
        }

        kDebug() << "firing";
        emit m_engine->ActionInvoked(dest[1].toUInt(), parameters()["actionId"].toString());
    }

    emitResult();
}

#include "notificationaction.moc"

