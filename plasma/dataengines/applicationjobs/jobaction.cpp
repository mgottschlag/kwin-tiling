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

#include "jobaction.h"
#include "kuiserverengine.h"

#include <kdebug.h>

void JobAction::start()
{
    kDebug() << "Trying to perform the action" << operationName();

    if (!m_jobView) {
        setErrorText(i18nc("%1 is the subject (can be anything) upon which the job is performed",
                           "The JobView for %1 cannot be found", destination()));
        setError(-1);
        emitResult();
        return;
    }

    //TODO: check with capabilities before performing actions.
    if (operationName() == "resume") {
        emit m_jobView->resumeRequested();
    } else if (operationName() == "suspend") {
        emit m_jobView->suspendRequested();
    } else if (operationName() == "stop") {
        emit m_jobView->cancelRequested();
    }

    emitResult();
}

#include "jobaction.moc"

