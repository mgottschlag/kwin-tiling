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
#include "core/leavemodel.h"

// KDE
#include <KLocalizedString>
#include <KIcon>
#include <solid/powermanagement.h>

// Local
#include "core/models.h"

using namespace Kickoff;

class LeaveModel::Private
{
};

LeaveModel::LeaveModel(QObject *parent)
    : QStandardItemModel(parent)
    , d(0)
{
    // Session Options
    QStandardItem *sessionOptions = new QStandardItem(i18n("Session"));

        // Logout
        QStandardItem *logoutOption = new QStandardItem(KIcon("system-log-out"),i18n("Logout"));
        logoutOption->setData("leave:/logout",Kickoff::UrlRole);
        logoutOption->setData(i18n("End session"),Kickoff::SubTitleRole);
        sessionOptions->appendRow(logoutOption);

        // Lock
        QStandardItem *lockOption = new QStandardItem(KIcon("system-lock-screen"),i18n("Lock"));
        lockOption->setData("leave:/lock",Kickoff::UrlRole);
        lockOption->setData(i18n("Lock the screen"),Kickoff::SubTitleRole);
        sessionOptions->appendRow(lockOption);

        // Switch User
        QStandardItem *switchUserOption = new QStandardItem(KIcon("switchuser"),i18n("Switch User"));
        switchUserOption->setData("leave:/switch",Kickoff::UrlRole);
        switchUserOption->setData(i18n("Start a parallel session as a different user."),Kickoff::SubTitleRole);
        sessionOptions->appendRow(switchUserOption);

    // System Options
    QStandardItem *systemOptions = new QStandardItem(i18n("System"));

        using namespace Solid::PowerManagement;
        QSet<SleepState> sleepStates = supportedSleepStates();

        // Sleep
        if (sleepStates.contains(SuspendState)) {
            QStandardItem *sleepOption = new QStandardItem(i18n("Sleep"));
            sleepOption->setData("leave:/sleep",Kickoff::UrlRole);
            systemOptions->appendRow(sleepOption);
        }

        // Hibernate
        if (sleepStates.contains(HibernateState)) {
            QStandardItem *hibernateOption = new QStandardItem(i18n("Hibernate"));
            hibernateOption->setData("leave:/hibernate",Kickoff::UrlRole);
            systemOptions->appendRow(hibernateOption);
        }

        // Shutdown
        QStandardItem *shutDownOption = new QStandardItem(i18n("Shutdown"));
        shutDownOption->setData("leave:/shutdown",Kickoff::UrlRole);
        shutDownOption->setData(i18n("Turn off the computer"),Kickoff::SubTitleRole);
        systemOptions->appendRow(shutDownOption);

        // Restart
        QStandardItem *restartOption = new QStandardItem(i18n("Restart"));
        restartOption->setData("leave:/restart",Kickoff::UrlRole);
        restartOption->setData(i18n("Restart the computer"),Kickoff::SubTitleRole);
        systemOptions->appendRow(restartOption);

    appendRow(sessionOptions);
    appendRow(systemOptions);
}
LeaveModel::~LeaveModel()
{
    delete d;
}

#include "leavemodel.moc"

