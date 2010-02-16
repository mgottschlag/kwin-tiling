/***************************************************************************
 *   completedjobnotification.h                                            *
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

#ifndef COMPLETEDJOBNOTIFICATION_H
#define COMPLETEDJOBNOTIFICATION_H

#include "notification.h"


class Job;

class CompletedJobNotification : public Notification
{
    Q_OBJECT

public:
    CompletedJobNotification(QObject *parent = 0);
    virtual ~CompletedJobNotification();

    void setJob(Job *job);
    Job *job() const;

public Q_SLOTS:
    void linkActivated(const QString &link);
    void triggerAction(const QString &actionId);

private:
    Job *m_job;
    QString m_destinationPrettyUrl;
};


#endif

