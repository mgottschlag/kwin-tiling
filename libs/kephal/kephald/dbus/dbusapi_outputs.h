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
#include <QVariant>
#include <QStringList>
#include <QSize>
#include <QPoint>


namespace kephal {
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
        QSize availableSize(QString id, int i);
        QPoint position(QString id);
        bool isConnected(QString id);
        bool isActivated(QString id);
        
    Q_SIGNALS:
        void outputConnected(QString id);
        void outputDisconnected(QString id);
        void outputActivated(QString id);
        void outputDeactivated(QString id);
        void outputResized(QString id);
        void outputMoved(QString id);
        
    private Q_SLOTS:
        void outputConnectedSlot(kephal::Output * o);
        void outputDisconnectedSlot(kephal::Output * o);
        void outputActivatedSlot(kephal::Output * o);
        void outputDeactivatedSlot(kephal::Output * o);
        void outputResizedSlot(kephal::Output * o, QSize oldSize, QSize newSize);
        void outputMovedSlot(kephal::Output * o, QPoint oldPosition, QPoint newPosition);
};


#endif // DBUSAPI_OUTPUTS_H

