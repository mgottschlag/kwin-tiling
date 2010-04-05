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


#ifndef DBUSAPI_OUTPUTS_H
#define DBUSAPI_OUTPUTS_H


#include <QObject>

#include "kephal.h"

#include <QSize>
#include <QPoint>
#include <QStringList>
#include <QMap>

namespace Kephal {
    class Output;
}

class DBusAPIOutputs : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Kephal.Outputs")

    public:
        DBusAPIOutputs(QObject * parent);

    public Q_SLOTS:
        QStringList outputIds();
        QSize size(QString id);
        int numAvailableSizes(QString id);
        QSize availableSize(QString id, int index);
        QPoint position(QString id);
        bool isConnected(QString id);
        bool isActivated(QString id);
        int rotation(QString id);
        double rate(QString id);
        bool reflectX(QString id);
        bool reflectY(QString id);
        int numAvailableRates(QString id);
        double availableRate(QString id, int index);

    Q_SIGNALS:
        void outputConnected(QString id);
        void outputDisconnected(QString id);
        void outputActivated(QString id);
        void outputDeactivated(QString id);
        void outputResized(QString id);
        void outputMoved(QString id);
        void outputRateChanged(QString id);
        void outputRotated(QString id);
        void outputReflected(QString id);

    private Q_SLOTS:
        void outputConnectedSlot(Kephal::Output * o);
        void outputDisconnectedSlot(Kephal::Output * o);
        void outputActivatedSlot(Kephal::Output * o);
        void outputDeactivatedSlot(Kephal::Output * o);
        void outputResizedSlot(Kephal::Output * o, QSize oldSize, QSize newSize);
        void outputMovedSlot(Kephal::Output * o, QPoint oldPosition, QPoint newPosition);
        void outputRateChangedSlot(Kephal::Output * o, float oldRate, float newRate);
        void outputRotatedSlot(Kephal::Output * o, Kephal::Rotation oldRotation, Kephal::Rotation newRotation);
        void outputReflectedSlot(Kephal::Output * o, bool oldX, bool oldY, bool newX, bool newY);

    private:
        QMap<QString, QList<QSize> > m_sizes;
        QMap<QString, QList<float> > m_rates;
};


#endif // DBUSAPI_OUTPUTS_H

