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


#include <QTimer>
#include <QStringList>
#include <kdebug.h>

#include "bluezcalljob.h"


BluezCallJob::BluezCallJob(const QDBusConnection &connection, const QString &dest,
                           const QString &path, const QString &interface,
                           const QString &methodName, const QList<QVariant> &params)
        : KJob(), m_connection(connection), m_dest(dest), m_path(path),
        m_iface(interface), m_method(methodName),
        m_params(params)
{}

BluezCallJob::~BluezCallJob()
{
}

void BluezCallJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void BluezCallJob::kill(bool /*quietly*/)
{
}

void BluezCallJob::doStart()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_dest, m_path,
                       m_iface, m_method);
    msg.setArguments(m_params);

    if (!m_connection.callWithCallback(msg, this, SLOT(callReply(const QDBusMessage&)))) {
        setError(1);
        setErrorText(m_connection.lastError().name() + ": " + m_connection.lastError().message());
        emitResult();
    }
}

void BluezCallJob::callReply(const QDBusMessage &reply)
{
    setError(0);

    if (reply.type() == QDBusMessage::InvalidMessage) {
        setError(1);
        setErrorText(m_connection.lastError().name() + ": " + m_connection.lastError().message());
    } else if (reply.type() == QDBusMessage::ErrorMessage) {
        setError(1);
        setErrorText(reply.interface() + ": " + reply.arguments().at(0).toString());
    }

    emitResult();
}

#include "bluezcalljob.moc"
