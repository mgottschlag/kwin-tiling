/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef BLUEZCALLJOB_H
#define BLUEZCALLJOB_H

#include <kjob.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QList>
#include <QVariant>

class BluezCallJob : public KJob
{
    Q_OBJECT
public:
    BluezCallJob(const QDBusConnection &connection, const QString &dest,
                 const QString &path, const QString &interface,  const QString &methodName,
                 const QList<QVariant> &params);
    virtual ~BluezCallJob();

    void start();
    void kill(bool quietly);

private Q_SLOTS:
    void doStart();
    void callError(const QDBusError &error);
    void callReply(const QDBusMessage &reply);

private:
    QDBusConnection m_connection;
    QString m_dest;
    QString m_path;
    QString m_iface;
    QString m_method;
    QList<QVariant> m_params;
};

#endif
