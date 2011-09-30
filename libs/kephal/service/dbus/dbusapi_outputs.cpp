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


#include "dbusapi_outputs.h"
#include "outputs.h"
#include "outputsadaptor.h"

#include <KDebug>


using namespace Kephal;

DBusAPIOutputs::DBusAPIOutputs(QObject * parent)
    : QObject(parent)
{
    new OutputsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();

    const bool result = dbus.registerObject("/modules/kephal/Outputs", this);
    kDebug() << "outputs registered on the bus:" << result;

    connect(Outputs::self(), SIGNAL(outputConnected(Kephal::Output*)), this, SLOT(outputConnectedSlot(Kephal::Output*)));
    connect(Outputs::self(), SIGNAL(outputDisconnected(Kephal::Output*)), this, SLOT(outputDisconnectedSlot(Kephal::Output*)));
    connect(Outputs::self(), SIGNAL(outputActivated(Kephal::Output*)), this, SLOT(outputActivatedSlot(Kephal::Output*)));
    connect(Outputs::self(), SIGNAL(outputDeactivated(Kephal::Output*)), this, SLOT(outputDeactivatedSlot(Kephal::Output*)));
    connect(Outputs::self(), SIGNAL(outputResized(Kephal::Output*,QSize,QSize)), this, SLOT(outputResizedSlot(Kephal::Output*,QSize,QSize)));
    connect(Outputs::self(), SIGNAL(outputMoved(Kephal::Output*,QPoint,QPoint)), this, SLOT(outputMovedSlot(Kephal::Output*,QPoint,QPoint)));
    connect(Outputs::self(), SIGNAL(outputRotated(Kephal::Output*,Kephal::Rotation,Kephal::Rotation)), this, SLOT(outputRotatedSlot(Kephal::Output*,Kephal::Rotation,Kephal::Rotation)));
    connect(Outputs::self(), SIGNAL(outputRateChanged(Kephal::Output*,float,float)), this, SLOT(outputRateChangedSlot(Kephal::Output*,float,float)));
    connect(Outputs::self(), SIGNAL(outputReflected(Kephal::Output*,bool,bool,bool,bool)), this, SLOT(outputReflectedSlot(Kephal::Output*,bool,bool,bool,bool)));
}

QSize DBusAPIOutputs::size(QString id)
{
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return output->size();
    }
    return QSize(0,0);
}

int DBusAPIOutputs::numAvailableSizes(QString id)
{
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        m_sizes.insert(id, output->availableSizes());
        return m_sizes[id].size();
    }
    return 0;
}

QSize DBusAPIOutputs::availableSize(QString id, int i)
{
    if (! m_sizes.contains(id)) {
        numAvailableSizes(id);
    }
    if (m_sizes.contains(id) && (m_sizes[id].size() > i)) {
        return m_sizes[id][i];
    }
    return QSize(-1, -1);
}

QPoint DBusAPIOutputs::position(QString id)
{
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return output->position();
    }
    return QPoint(0,0);
}

QStringList DBusAPIOutputs::outputIds()
{
    QList<Output *> outputs = Outputs::self()->outputs();
    QStringList result;
    //kDebug() << "output-ids requested!!";
    foreach (Output * output, outputs) {
        //kDebug() << "appending output-id:" << output->id();
        result.append(output->id());
    }
    return result;
}

bool DBusAPIOutputs::isConnected(QString id)
{
    Output * output = Outputs::self()->output(id);
    if (output && output->isConnected()) {
        return true;
    }
    return false;
}

bool DBusAPIOutputs::isActivated(QString id)
{
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return true;
    }
    return false;
}

int DBusAPIOutputs::numAvailableRates(QString id)
{
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        m_rates.insert(id, output->availableRates());
        return m_rates[id].size();
    }
    return 0;
}

double DBusAPIOutputs::availableRate(QString id, int i)
{
    if (! m_rates.contains(id)) {
        numAvailableRates(id);
    }
    if (m_rates.contains(id) && (m_rates[id].size() > i)) {
        return m_rates[id][i];
    }
    return 0;
}

int DBusAPIOutputs::rotation(QString id) {
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return output->rotation();
    }
    return RotateNormal;
}

double DBusAPIOutputs::rate(QString id) {
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return output->rate();
    }
    return 0;
}

bool DBusAPIOutputs::reflectX(QString id) {
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return output->reflectX();
    }
    return false;
}

bool DBusAPIOutputs::reflectY(QString id) {
    Output * output = Outputs::self()->output(id);
    if (output && output->isActivated()) {
        return output->reflectY();
    }
    return false;
}

void DBusAPIOutputs::outputConnectedSlot(Kephal::Output * o) {
    emit outputConnected(o->id());
}

void DBusAPIOutputs::outputDisconnectedSlot(Kephal::Output * o) {
    emit outputDisconnected(o->id());
}

void DBusAPIOutputs::outputActivatedSlot(Kephal::Output * o) {
    emit outputActivated(o->id());
}

void DBusAPIOutputs::outputDeactivatedSlot(Kephal::Output * o) {
    emit outputDeactivated(o->id());
}

void DBusAPIOutputs::outputResizedSlot(Kephal::Output * o, QSize oldSize, QSize newSize) {
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)
    emit outputResized(o->id());
}

void DBusAPIOutputs::outputMovedSlot(Kephal::Output * o, QPoint oldPosition, QPoint newPosition) {
    Q_UNUSED(oldPosition)
    Q_UNUSED(newPosition)
    emit outputMoved(o->id());
}

void DBusAPIOutputs::outputRateChangedSlot(Kephal::Output * o, float oldRate, float newRate) {
    Q_UNUSED(oldRate)
    Q_UNUSED(newRate)
    emit outputRateChanged(o->id());
}

void DBusAPIOutputs::outputRotatedSlot(Kephal::Output * o, Rotation oldRotation, Rotation newRotation) {
    Q_UNUSED(oldRotation)
    Q_UNUSED(newRotation)
    emit outputRotated(o->id());
}

void DBusAPIOutputs::outputReflectedSlot(Kephal::Output * o, bool oldX, bool oldY, bool newX, bool newY) {
    Q_UNUSED(oldX)
    Q_UNUSED(oldY)
    Q_UNUSED(newX)
    Q_UNUSED(newY)
    emit outputReflected(o->id());
}

#ifndef NO_KDE
#include "dbusapi_outputs.moc"
#endif

