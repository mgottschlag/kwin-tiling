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


#include "dbusoutputs.h"

#include <QDebug>

#include "simpleoutput.h"


namespace Kephal {

    DBusOutputs::DBusOutputs(QObject * parent)
            : Outputs(parent)
    {
        m_interface = new org::kde::Kephal::Outputs(
            "org.kde.Kephal",
            "/modules/kephal/Outputs",
            QDBusConnection::sessionBus(),
            this);

        if (! m_interface->isValid()) {
            m_valid = false;
            return;
        }

        m_valid = true;

        const QStringList ids = m_interface->outputIds();
        foreach (const QString& id, ids) {
            QPoint pos = m_interface->position(id);
            QSize size = m_interface->size(id);
            bool connected = m_interface->isConnected(id);
            bool activated = m_interface->isActivated(id);
            //qDebug() << "adding an output" << id << "with geom: " << pos << size;

            SimpleOutput * output = new SimpleOutput(this,
                    id,
                    size,
                    pos,
                    connected,
                    activated);

            m_outputs.append(output);

            if (connected) {
                output->_setRate(m_interface->rate(id));
                int rotation = m_interface->rotation(id);
                output->_setRotation((Rotation) rotation);
                output->_setReflectX(m_interface->reflectX(id));
                output->_setReflectY(m_interface->reflectY(id));

                outputConnectedSlot(id);
            }
        }

        connect(m_interface, SIGNAL(outputConnected(QString)), this, SLOT(outputConnectedSlot(QString)));
        connect(m_interface, SIGNAL(outputDisconnected(QString)), this, SLOT(outputDisconnectedSlot(QString)));
        connect(m_interface, SIGNAL(outputActivated(QString)), this, SLOT(outputActivatedSlot(QString)));
        connect(m_interface, SIGNAL(outputDeactivated(QString)), this, SLOT(outputDeactivatedSlot(QString)));
        connect(m_interface, SIGNAL(outputResized(QString)), this, SLOT(outputResizedSlot(QString)));
        connect(m_interface, SIGNAL(outputMoved(QString)), this, SLOT(outputMovedSlot(QString)));
        connect(m_interface, SIGNAL(outputRotated(QString)), this, SLOT(outputRotatedSlot(QString)));
        connect(m_interface, SIGNAL(outputRateChanged(QString)), this, SLOT(outputRateChangedSlot(QString)));
        connect(m_interface, SIGNAL(outputReflected(QString)), this, SLOT(outputReflectedSlot(QString)));
    }

    QList<Output *> DBusOutputs::outputs()
    {
        QList<Output *> result;
        foreach(SimpleOutput * output, m_outputs) {
            result.append(output);
        }
        return result;
    }

    void DBusOutputs::activateLayout(const QMap<Output *, QRect> & layout)
    {
        Q_UNUSED(layout)
    }

    bool DBusOutputs::isValid() {
        return m_valid;
    }

    void DBusOutputs::outputConnectedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            o->_setConnected(true);

            int numSizes = m_interface->numAvailableSizes(id);
            QList<QSize> sizes;
            for (int i = 0; i < numSizes; ++i) {
                sizes << m_interface->availableSize(id, i);
            }
            o->_setAvailableSizes(sizes);

            int numRates = m_interface->numAvailableRates(id);
            QList<float> rates;
            for (int i = 0; i < numRates; ++i) {
                rates << m_interface->availableRate(id, i);
            }
            o->_setAvailableRates(rates);

            emit outputConnected(o);
        }
    }

    void DBusOutputs::outputDisconnectedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            o->_setConnected(false);
            emit outputDisconnected(o);
        }
    }

    void DBusOutputs::outputActivatedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            o->_setActivated(true);
            o->_setSize(m_interface->size(id));
            emit outputActivated(o);
        }
    }

    void DBusOutputs::outputDeactivatedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            o->_setActivated(false);
            emit outputDeactivated(o);
        }
    }

    void DBusOutputs::outputResizedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            QSize prev = o->size();
            o->_setSize(m_interface->size(id));
            emit outputResized(o, prev, o->size());
        }
    }

    void DBusOutputs::outputMovedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            QPoint prev = o->position();
            o->_setPosition(m_interface->position(id));
            emit outputMoved(o, prev, o->position());
        }
    }

    void DBusOutputs::outputRotatedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            Rotation prev = o->rotation();
            int rotation = m_interface->rotation(id);
            o->_setRotation((Rotation) rotation);
            emit outputRotated(o, prev, o->rotation());
        }
    }

    void DBusOutputs::outputRateChangedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            float prev = o->rate();
            o->_setRate(m_interface->rate(id));
            emit outputRateChanged(o, prev, o->rate());
        }
    }

    void DBusOutputs::outputReflectedSlot(QString id) {
        SimpleOutput * o = (SimpleOutput *) output(id);
        if (o) {
            bool prevX = o->reflectX();
            bool prevY = o->reflectY();
            o->_setReflectX(m_interface->reflectX(id));
            o->_setReflectY(m_interface->reflectY(id));
            emit outputReflected(o, prevX, prevY, o->reflectX(), o->reflectY());
        }
    }

}

