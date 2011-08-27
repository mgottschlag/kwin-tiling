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


#include "dbusapi_configurations.h"
#include "configurations.h"
#include "configurationsadaptor.h"
#include "outputs.h"

#include <KDebug>
#include <QObject>


using namespace Kephal;

DBusAPIConfigurations::DBusAPIConfigurations(QObject * parent)
    : QObject(parent)
{
    new ConfigurationsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();

    const bool result = dbus.registerObject("/modules/kephal/Configurations", this);
    kDebug() << "configurations registered on the bus:" << result;

    connect(Configurations::self(), SIGNAL(configurationActivated(Kephal::Configuration*)), this, SLOT(configurationActivatedSlot(Kephal::Configuration*)));
    connect(Configurations::self(), SIGNAL(confirmed()), this, SIGNAL(confirmed()));
    connect(Configurations::self(), SIGNAL(reverted()), this, SIGNAL(reverted()));
    connect(Configurations::self(), SIGNAL(confirmTimeout(int)), this, SIGNAL(confirmTimeout(int)));
}

QStringList DBusAPIConfigurations::configurations() {
    QStringList result;
    foreach (const QString& name, Configurations::self()->configurations().keys()) {
        result << name;
    }
    return result;
}

int DBusAPIConfigurations::numAvailablePositions(QString output) {
    Output * o = Outputs::self()->output(output);
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
    foreach (Configuration * config, Configurations::self()->alternateConfigurations()) {
        result << config->name();
    }
    return result;
}

QString DBusAPIConfigurations::activeConfiguration() {
    Configuration * config = Configurations::self()->activeConfiguration();
    if (config) {
        return config->name();
    }
    return "";
}

// Not needed. If Configurations provides access to configurations, it should not also provide ways
// to temporarily modify the current configuration.  This should be done by a config UI.
#if 0
bool DBusAPIConfigurations::move(QString output, QPoint position) {
    Output * o = Outputs::self()->output(output);
    if (o) {
        return Configurations::self()->move(o, position);
    }
    return false;
}

bool DBusAPIConfigurations::resize(QString output, QSize size) {
    Output * o = Outputs::self()->output(output);
    if (o) {
        return Configurations::self()->resize(o, size);
    }
    return false;
}

bool DBusAPIConfigurations::rotate(QString output, int rotation) {
    Output * o = Outputs::self()->output(output);
    if (o) {
        return Configurations::self()->rotate(o, (Rotation) rotation);
    }
    return false;
}

bool DBusAPIConfigurations::changeRate(QString output, double rate) {
    Output * o = Outputs::self()->output(output);
    if (o) {
        return Configurations::self()->changeRate(o, static_cast<float>(rate));
    }
    return false;
}

bool DBusAPIConfigurations::reflectX(QString output, bool reflect) {
    Output * o = Outputs::self()->output(output);
    if (o) {
        return Configurations::self()->reflectX(o, reflect);
    }
    return false;
}

bool DBusAPIConfigurations::reflectY(QString output, bool reflect) {
    Output * o = Outputs::self()->output(output);
    if (o) {
        return Configurations::self()->reflectY(o, reflect);
    }
    return false;
}
#endif
bool DBusAPIConfigurations::isModifiable(QString config) {
    Configuration * c = Configurations::self()->configuration(config);
    if (c) {
        return c->isModifiable();
    }
    return false;
}

bool DBusAPIConfigurations::isActivated(QString config) {
    Configuration * c = Configurations::self()->configuration(config);
    if (c) {
        return c->isActivated();
    }
    return false;
}

void DBusAPIConfigurations::activate(QString config) {
    Configuration * c = Configurations::self()->configuration(config);
    if (c) {
        c->activate();
    }
}

int DBusAPIConfigurations::primaryScreen(QString config) {
    Configuration * c = Configurations::self()->configuration(config);
    if (c) {
        return c->primaryScreen();
    }
    return 0;
}

int DBusAPIConfigurations::screen(QString outputId) {
    Output * output = Outputs::self()->output(outputId);
    if (output) {
        return Configurations::self()->screen(output);
    }
    return -1;
}

void DBusAPIConfigurations::setPolling(bool polling) {
    Configurations::self()->setPolling(polling);
}

bool DBusAPIConfigurations::polling() {
    return Configurations::self()->polling();
}

void DBusAPIConfigurations::configurationActivatedSlot(Kephal::Configuration * configuration) {
    emit configurationActivated(configuration->name());
}

void DBusAPIConfigurations::confirm() {
    Configurations::self()->confirm();
}

void DBusAPIConfigurations::revert() {
    Configurations::self()->revert();
}

#ifndef NO_KDE
#include "dbusapi_configurations.moc"
#endif

