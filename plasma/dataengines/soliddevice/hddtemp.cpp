/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
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

#include "hddtemp.h"

#include <QTcpSocket>

#include <KDebug>

HddTemp::HddTemp(QObject* parent)
    : QObject(parent),
      m_failCount(0),
      m_timer(0)
{
    updateData();
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    m_timer->start(10000);
}

HddTemp::~HddTemp()
{
}

QStringList HddTemp::sources() const
{
    return m_data.keys();
}

bool HddTemp::updateData()
{
    if (m_failCount > 4) {
        return false;
    }
    QTcpSocket socket;
    QString data;

    socket.connectToHost("localhost", 7634);
    if (socket.waitForConnected(500)) {
        while (data.length() < 1024) {
            if (!socket.waitForReadyRead(500)) {
                if (data.length() > 0) {
                    break;
                } else {
                    //kDebug() << socket.errorString();
                    return false;
                }
            }
            data += QString(socket.readAll());
        }
        socket.disconnectFromHost();
        //on success retry fail count
        m_failCount = 0;
    } else {
        m_failCount++;
        //kDebug() << socket.errorString();
        return false;
    }
    QStringList list = data.split('|');
    int i = 1;
    m_data.clear();
    while (i + 4 < list.size()) {
        m_data[list[i]].append(list[i + 2]);
        m_data[list[i]].append(list[i + 3]);
        i += 5;
    }
    return true;
}

QVariant HddTemp::data(const QString source, const DataType type) const
{
    return m_data[source][type];
}

#include "hddtemp.moc"
