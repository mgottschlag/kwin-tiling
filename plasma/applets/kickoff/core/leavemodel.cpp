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

// Qt
#include <QFileInfo>

// KDE
#include <KDebug>
#include <KLocalizedString>
#include <KIcon>
#include <solid/control/powermanager.h>
#include <kworkspace/kworkspace.h>

// Local
#include "core/models.h"

using namespace Kickoff;

class LeaveModel::Private
{
};

QStandardItem* LeaveModel::createStandardItem(const QString& url)
{
    //Q_ASSERT(KUrl(url).scheme() == "leave");
    QStandardItem *item = new QStandardItem();
    const QString basename = QFileInfo(url).baseName();
    kDebug() << "basename is" << basename;
    if (basename == "logout") {
        item->setText(i18n("Logout"));
        item->setIcon(KIcon("system-log-out"));
        item->setData(i18n("End session"),Kickoff::SubTitleRole);
    } else if (basename == "lock") {
        item->setText(i18n("Lock"));
        item->setIcon(KIcon("system-lock-screen"));
        item->setData(i18n("Lock the screen"),Kickoff::SubTitleRole);
    } else if (basename == "switch") {
        item->setText(i18n("Switch User"));
        item->setIcon(KIcon("system-switch-user"));
        item->setData(i18n("Start a parallel session as a different user"),Kickoff::SubTitleRole);
    } else if (basename == "sleep") {
        item->setText(i18n("Sleep"));
        item->setIcon(KIcon("system-suspend"));
        item->setData(i18n("Suspend to RAM"), Kickoff::SubTitleRole);
    } else if (basename == "hibernate") {
        item->setText(i18n("Hibernate"));
        item->setIcon(KIcon("system-suspend-hibernate"));
        item->setData(i18n("Suspend to disk"), Kickoff::SubTitleRole);
    } else if (basename == "shutdown") {
        item->setText(i18n("Shutdown"));
        item->setIcon(KIcon("system-shutdown"));
        item->setData(i18n("Turn off the computer"),Kickoff::SubTitleRole);
    } else if (basename == "restart") {
        item->setText(i18n("Restart"));
        item->setIcon(KIcon("system-restart"));
        item->setData(i18n("Restart the computer"),Kickoff::SubTitleRole);
    } else {
        item->setText(basename);
        item->setData(url,Kickoff::SubTitleRole);
    }
    item->setData(url,Kickoff::UrlRole);
    return item;
}

LeaveModel::LeaveModel(QObject *parent)
    : QStandardItemModel(parent)
    , d(0)
{
    // Session Options
    QStandardItem *sessionOptions = new QStandardItem(i18n("Session"));

    // Logout
    QStandardItem *logoutOption = createStandardItem("leave:/logout");
    sessionOptions->appendRow(logoutOption);

    // Lock
    QStandardItem *lockOption = createStandardItem("leave:/lock");
    sessionOptions->appendRow(lockOption);

    // Switch User
    QStandardItem *switchUserOption = createStandardItem("leave:/switch");
    sessionOptions->appendRow(switchUserOption);

    // System Options
    QStandardItem *systemOptions = new QStandardItem(i18n("System"));
    bool addSystemSession = false;

    {
        using namespace Solid::Control::PowerManager;
        SuspendMethods sleepStates = supportedSuspendMethods();

        // Sleep
        if (sleepStates & ToRam) {
            QStandardItem *sleepOption = createStandardItem("leave:/sleep");
            systemOptions->appendRow(sleepOption);
            addSystemSession = true;
        }

        // Hibernate
        if (sleepStates & ToDisk) {
            QStandardItem *hibernateOption = createStandardItem("leave:/hibernate");
            systemOptions->appendRow(hibernateOption);
            addSystemSession = true;
        }
    }

//FIXME: the proper fix is to implement the KWorkSpace methods for Windows
#ifndef Q_WS_WIN
    if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt)) {
        // Shutdown
        QStandardItem *shutDownOption = createStandardItem("leave:/shutdown");
        systemOptions->appendRow(shutDownOption);
        addSystemSession = true;
    }

    if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot)) {
        // Restart
        QStandardItem *restartOption = createStandardItem("leave:/restart");
        systemOptions->appendRow(restartOption);
        addSystemSession = true;
    }
#endif

    appendRow(sessionOptions);
    if (addSystemSession) {
       appendRow(systemOptions);
    } else {
        delete systemOptions;
    }
}

LeaveModel::~LeaveModel()
{
    delete d;
}

#include "leavemodel.moc"

