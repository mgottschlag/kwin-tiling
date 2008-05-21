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

#ifndef HDDTEMP_H
#define HDDTEMP_H

#include <QMap>
#include <QTcpSocket>

class QObject;
class QString;
class QStringList;
class QVariant;
class QTimer;
class QTcpSocket;


class HddTemp : public QObject
{
    Q_OBJECT

    public:
        enum DataType {Temperature=0, Unit};
        
        HddTemp(QObject *parent=0);
        ~HddTemp();
        QStringList sources() const;
        QVariant data(const QString source, const DataType type) const;

    private Q_SLOTS:
        void updateData();
        void onConnected();
        void onReadReady();
        void onReadComplete();
        void onError();
        
    private:
        int m_failCount;
        QTcpSocket m_socket;
        QString m_bufferedData;
        QMap<QString, QList<QVariant> > m_data;
        QTimer *m_timer;
};


#endif
