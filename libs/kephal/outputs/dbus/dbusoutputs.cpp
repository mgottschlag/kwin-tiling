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


#include <QApplication>
#include <QDebug>
#include <QVariant>

#include "dbusoutputs.h"
#include "../simpleoutput.h"
#include "outputs_interface.h"


namespace kephal {

    DBusOutputs::DBusOutputs(QObject * parent)
            : Outputs(parent)
    {
        m_interface = new org::kde::Kephal::Outputs(
            "org.kde.Kephal",
            "/Outputs",
            QDBusConnection::sessionBus(),
            this);
            
        if (! m_interface->isValid()) {
            m_valid = false;
            return;
        }
            
        m_valid = true;
        
        QStringList ids = m_interface->outputIds();
        foreach (QString id, ids) {
            //QString id = varId.toString();
            
            QPoint pos = m_interface->position(id);
            QSize size = m_interface->size(id);
            bool connected = m_interface->isConnected(id);
            bool activated = m_interface->isActivated(id);
            qDebug() << "adding an output" << id << "with geom: " << pos << size;
            
            SimpleOutput * output = new SimpleOutput(this,
                    id,
                    size,
                    pos,
                    connected,
                    activated);
                    
            int numSizes = m_interface->numAvailableSizes(id);
            QList<QSize> sizes;
            for (int i = 0; i < numSizes; ++i) {
                QSize size = m_interface->availableSize(id, i);
                //qDebug() << "size from dbus:" << size;
                sizes.append(size);
            }
            output->_setAvailableSizes(sizes);
            
            connect(output, SIGNAL(sizeChangeRequested(SimpleOutput *, QSize, QSize)), this, SLOT(resizeRequested(SimpleOutput *, QSize, QSize)));
            m_outputs.append(output);
        }
    }
    
    DBusOutputs::~DBusOutputs() {
        foreach(Output * output, m_outputs) {
            delete output;
        }
        m_outputs.clear();
    }

    void DBusOutputs::resizeRequested(SimpleOutput * output, QSize oldSize, QSize newSize) {
        qDebug() << "resize requested:" << output->id() << oldSize << newSize;
        m_interface->setSize(output->id(), newSize);
    }
    
    QList<Output *> DBusOutputs::outputs()
    {
        QList<Output *> result;
        foreach(SimpleOutput * output, m_outputs) {
            result.append(output);
        }
        return result;
    }
    
    void DBusOutputs::activateLayout(QMap<Output *, QRect> layout)
    {
    }
    
    bool DBusOutputs::isValid() {
        return m_valid;
    }
    
}

