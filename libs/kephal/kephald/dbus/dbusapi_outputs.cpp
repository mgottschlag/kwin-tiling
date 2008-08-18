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


#include <QDebug>

#include "dbusapi_outputs.h"
#include "outputs/outputs.h"
#include "outputsadaptor.h"

#include <QVariant>


using namespace kephal;

DBusAPIOutputs::DBusAPIOutputs(QObject * parent)
    : QObject(parent)
{
    new OutputsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    
    bool result;
    result = dbus.registerObject("/Outputs", this);
    qDebug() << "outputs registered on the bus:" << result;
}

QSize DBusAPIOutputs::size(QString id)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isActivated()) {
        return output->size();
    }
    return QSize(0,0);
}

void DBusAPIOutputs::setSize(QString id, QSize size)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isActivated()) {
        output->setSize(size);
    }
}

int DBusAPIOutputs::numAvailableSizes(QString id)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isActivated()) {
        return output->availableSizes().size();
    }
    return 0;
}

QSize DBusAPIOutputs::availableSize(QString id, int i)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isActivated()) {
        return output->availableSizes()[i];
    }
    return QSize();
}

QPoint DBusAPIOutputs::position(QString id)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isActivated()) {
        return output->position();
    }
    return QPoint(0,0);
}

QStringList DBusAPIOutputs::outputIds()
{
    QList<Output *> outputs = Outputs::instance()->outputs();
    QStringList result;
    //qDebug() << "output-ids requested!!";
    foreach (Output * output, outputs) {
        //qDebug() << "appending output-id:" << output->id();
        result.append(output->id());
    }
    return result;
}

bool DBusAPIOutputs::isConnected(QString id)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isConnected()) {
        return true;
    }
    return false;
}

bool DBusAPIOutputs::isActivated(QString id)
{
    Output * output = Outputs::instance()->output(id);
    if (output && output->isActivated()) {
        return true;
    }
    return false;
}

/*Output * DBusAPIOutputs::output(QString id) {
    QList<Output *> outputs = Outputs::instance()->outputs();
    foreach (Output * output, outputs) {
        if (output->id() == id) {
            return output;
        }
    }
    return 0;
}*/


