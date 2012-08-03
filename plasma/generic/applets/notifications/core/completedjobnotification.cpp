/***************************************************************************
 *   completedjobnotification.h                                                          *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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

#include "completedjobnotification.h"
#include "job.h"

#include <QProcess>

#include <KIcon>
#include <KLocale>
#include <KDebug>


static const int completedJobExpireDelay = 60 * 1000;
static const int completedShortJobExpireDelay = 8 * 1000;
static const uint shortJobsLength = 30 * 1000;

CompletedJobNotification::CompletedJobNotification(QObject *parent)
    : Notification(parent)
{
}

CompletedJobNotification::~CompletedJobNotification()
{
}

void CompletedJobNotification::setJob(Job *job)
{
    setApplicationName(job->applicationName());
    setApplicationIcon(KIcon(job->applicationIconName()));
    setSummary(i18n("%1 [Finished]", job->message()));

    if (job->error().isEmpty()) {
        setMessage(job->completedMessage());
    } else {
        setMessage(job->error());
    }

    if (job->elapsed() < shortJobsLength) {
        setTimeout(completedShortJobExpireDelay);
    } else {
        setTimeout(completedJobExpireDelay);
    }

    if (job->destination().isValid()) {
        QHash<QString, QString> actions;
        actions.insert("open", i18n("Open"));
        setActions(actions);
        setActionOrder(QStringList()<<"open");

        // create location url as is done in job->completedMessage()
        KUrl location(job->destination());
        if (job->totalAmounts().value("files") > 1) {
            location.setFileName(QString());
        }

        m_destinationPrettyUrl = location.prettyUrl();
    }

    m_job = job;
}

void CompletedJobNotification::linkActivated(const QString &url)
{
    kDebug() << "open " << url;
    QProcess::startDetached("kde-open", QStringList() << url);
}

Job *CompletedJobNotification::job() const
{
    return m_job;
}

void CompletedJobNotification::triggerAction(const QString &actionId)
{
    if (actionId == "open" && !m_destinationPrettyUrl.isNull()) {
        linkActivated(m_destinationPrettyUrl);
    }
}


#include "completedjobnotification.moc"
