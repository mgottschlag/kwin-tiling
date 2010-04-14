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
#include <QStringList>
#include <QPoint>
#include <QSize>
#include <QMap>

namespace Kephal {
    class Configuration;
}


class DBusAPIConfigurations : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Kephal.Configurations")

    public:
        DBusAPIConfigurations(QObject * parent);

    public Q_SLOTS:
        QStringList configurations();
        QStringList alternateConfigurations();
        QString activeConfiguration();

        int numAvailablePositions(QString output);
        QPoint availablePosition(QString output, int index);
        //bool move(QString output, QPoint position);
        //bool resize(QString output, QSize size);
        //bool rotate(QString output, int rotation);
        //bool changeRate(QString output, double rate);
        //bool reflectX(QString output, bool reflect);
        //bool reflectY(QString output, bool reflect);
        int screen(QString output);

        bool isModifiable(QString config);
        bool isActivated(QString config);
        void activate(QString config);
        int primaryScreen(QString config);

        void setPolling(bool polling);
        bool polling();

        void confirm();
        void revert();

    Q_SIGNALS:
        void configurationActivated(QString name);
        void confirmTimeout(int seconds);
        void confirmed();
        void reverted();

    private Q_SLOTS:
        void configurationActivatedSlot(Kephal::Configuration * configuration);

    private:
        QMap<QString, QList<QPoint> > m_outputAvailablePositions;
};


#endif // DBUSAPI_CONFIGURATIONS_H

