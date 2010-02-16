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

#ifndef DBUSJOB_H
#define DBUSJOB_H

#include "../../core/job.h"



class DBusJob : public Job
{
    Q_OBJECT

    friend class DBusJobProtocol;

public:
    DBusJob(const QString &source, QObject *parent = 0);
    ~DBusJob();

public slots:
    void suspend();
    void resume();
    void stop();

signals:
    void jobDeleted(const QString &source);
    void suspend(const QString &source);
    void resume(const QString &source);
    void stop(const QString &source);

private:
    QString m_source;
};



#endif
