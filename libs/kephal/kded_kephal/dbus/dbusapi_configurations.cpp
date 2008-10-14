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
#include "kephal/configurations.h"
#include "configurationsadaptor.h"
#include "kephal/outputs.h"

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
    
    //connect(Configurations::instance(), SIGNAL(statusChanged(kephal::StatusMessage *)), this, SLOT(statusChangedSlot(kephal::StatusMessage *)));
    connect(Configurations::instance(), SIGNAL(configurationActivated(kephal::Configuration *)), this, SLOT(configurationActivatedSlot(kephal::Configuration *)));
    connect(Configurations::instance(), SIGNAL(confirmed()), this, SIGNAL(confirmed()));
    connect(Configurations::instance(), SIGNAL(reverted()), this, SIGNAL(reverted()));
    connect(Configurations::instance(), SIGNAL(confirmTimeout(int)), this, SIGNAL(confirmTimeout(int)));
}

QStringList DBusAPIConfigurations::configurations() {
    QStringList result;
    foreach (const QString& name, Configurations::instance()->configurations().keys()) {
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

QString DBusAPIConfigurations::activeConfiguration() {
    Configuration * config = Configurations::instance()->activeConfiguration();
    if (config) {
        return config->name();
    }
    return "";
}

bool DBusAPIConfigurations::move(QString output, QPoint position) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        return Configurations::instance()->move(o, position);
    }
    return false;
}

bool DBusAPIConfigurations::resize(QString output, QSize size) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        return Configurations::instance()->resize(o, size);
    }
    return false;
}

bool DBusAPIConfigurations::rotate(QString output, int rotation) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        return Configurations::instance()->rotate(o, (Rotation) rotation);
    }
    return false;
}

bool DBusAPIConfigurations::changeRate(QString output, qreal rate) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        return Configurations::instance()->changeRate(o, rate);
    }
    return false;
}

bool DBusAPIConfigurations::reflectX(QString output, bool reflect) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        return Configurations::instance()->reflectX(o, reflect);
    }
    return false;
}

bool DBusAPIConfigurations::reflectY(QString output, bool reflect) {
    Output * o = Outputs::instance()->output(output);
    if (o) {
        return Configurations::instance()->reflectY(o, reflect);
    }
    return false;
}

bool DBusAPIConfigurations::isModifiable(QString config) {
    Configuration * c = Configurations::instance()->configuration(config);
    if (c) {
        return c->isModifiable();
    }
    return false;
}

bool DBusAPIConfigurations::isActivated(QString config) {
    Configuration * c = Configurations::instance()->configuration(config);
    if (c) {
        return c->isActivated();
    }
    return false;
}

void DBusAPIConfigurations::activate(QString config) {
    Configuration * c = Configurations::instance()->configuration(config);
    if (c) {
        c->activate();
    }
}

int DBusAPIConfigurations::primaryScreen(QString config) {
    Configuration * c = Configurations::instance()->configuration(config);
    if (c) {
        return c->primaryScreen();
    }
    return 0;
}

int DBusAPIConfigurations::screen(QString outputId) {
    Output * output = Outputs::instance()->output(outputId);
    if (output) {
        return Configurations::instance()->screen(output);
    }
    return -1;
}

void DBusAPIConfigurations::setPolling(bool polling) {
    Configurations::instance()->setPolling(polling);
}

bool DBusAPIConfigurations::polling() {
    return Configurations::instance()->polling();
}

/*int DBusAPIConfigurations::statusType() {
    return Configurations::instance()->status()->type();
}

int DBusAPIConfigurations::statusMessage() {
    return Configurations::instance()->status()->message();
}

QString DBusAPIConfigurations::statusDescription() {
    return Configurations::instance()->status()->description();
}

void DBusAPIConfigurations::statusChangedSlot(kephal::StatusMessage * status) {
    Q_UNUSED(status)
    emit statusChanged();
}*/

void DBusAPIConfigurations::configurationActivatedSlot(kephal::Configuration * configuration) {
    emit configurationActivated(configuration->name());
}

void DBusAPIConfigurations::confirm() {
    Configurations::instance()->confirm();
}

void DBusAPIConfigurations::revert() {
    Configurations::instance()->revert();
}

#ifndef NO_KDE
#include "dbusapi_configurations.moc"
#endif

