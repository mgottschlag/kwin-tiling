/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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


#ifndef DBUSAPI_CONFIGURATIONS_H
#define DBUSAPI_CONFIGURATIONS_H


#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QPoint>
#include <QSize>


class DBusAPIConfigurations : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Kephal.Configurations")
    
    public:
        DBusAPIConfigurations(QObject * parent);
        
    public Q_SLOTS:
        QStringList configurations();
        QStringList alternateConfigurations();
        QString findConfiguration();
        QString activeConfiguration();

        int numAvailablePositions(QString output);
        QPoint availablePosition(QString output, int index);
        void move(QString output, QPoint position);
        void resize(QString output, QSize size);
        
        bool isModifiable(QString config);
        bool isActivated(QString config);
        void activate(QString config);
        
    private:
        QMap<QString, QList<QPoint> > m_outputAvailablePositions;
};


#endif // DBUSAPI_CONFIGURATIONS_H

