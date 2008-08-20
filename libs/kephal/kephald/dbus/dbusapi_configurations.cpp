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
#include <QObject>

#include "dbusapi_configurations.h"
#include "configurations/configurations.h"
#include "configurationsadaptor.h"
#include "outputs/outputs.h"

#include <QVariant>


using namespace kephal;

DBusAPIConfigurations::DBusAPIConfigurations(QObject * parent)
    : QObject(parent)
{
    new ConfigurationsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    
    bool result;
    result = dbus.registerObject("/Configurations", this);
    qDebug() << "configurations registered on the bus:" << result;
}

QStringList DBusAPIConfigurations::configurations() {
    QStringList result;
    foreach (QString name, Configurations::instance()->configurations().keys()) {
        result << name;
    }
    return result;
}

int DBusAPIConfigurations::numAvailablePositions(QString output) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        m_outputAvailablePositions.insert(output, o->availablePositions());
        return m_outputAvailablePositions[output].size();
    }
    return 0;
}

QPoint DBusAPIConfigurations::availablePosition(QString output, int index) {
    if (m_outputAvailablePositions.contains(output)) {
        return m_outputAvailablePositions[output][index];
    }
    return QPoint();
}

QStringList DBusAPIConfigurations::alternateConfigurations() {
    QStringList result;
    foreach (Configuration * config, Configurations::instance()->alternateConfigurations()) {
        result << config->name();
    }
    return result;
}

QString DBusAPIConfigurations::findConfiguration() {
    Configuration * config = Configurations::instance()->findConfiguration();
    if (config) {
        return config->name();
    }
    return "";
}

QString DBusAPIConfigurations::activeConfiguration() {
    Configuration * config = Configurations::instance()->activeConfiguration();
    if (config) {
        return config->name();
    }
    return "";
}

void DBusAPIConfigurations::move(QString output, QPoint position) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        Configurations::instance()->move(o, position);
    }
}

void DBusAPIConfigurations::resize(QString output, QSize size) {
    qDebug() << "DBusAPIConfigurations::resize() called" << output << size;
    Output * o = Outputs::instance()->output(output);
    if (o) {
        Configurations::instance()->resize(o, size);
    }
}

bool DBusAPIConfigurations::isModifiable(QString config) {
    Configuration * c = Configurations::instance()->activeConfiguration();
    if (c) {
        return c->isModifiable();
    }
    return false;
}

bool DBusAPIConfigurations::isActivated(QString config) {
    Configuration * c = Configurations::instance()->activeConfiguration();
    if (c) {
        return c->isActivated();
    }
    return false;
}

void DBusAPIConfigurations::activate(QString config) {
    Configuration * c = Configurations::instance()->activeConfiguration();
    if (c) {
        c->activate();
    }
}

