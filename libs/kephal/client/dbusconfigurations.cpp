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


#include "dbusconfigurations.h"

#include "outputs.h"


namespace Kephal {

    DBusConfiguration::DBusConfiguration(DBusConfigurations * parent, QString name)
        : Configuration(parent),
        m_name(name),
        m_parent(parent)
    {
    }

    QString DBusConfiguration::name() {
        return m_name;
    }

    bool DBusConfiguration::isModifiable() {
        return m_parent->interface()->isModifiable(m_name);
    }

    bool DBusConfiguration::isActivated() {
        return m_parent->interface()->isActivated(m_name);
    }

    QMap<int, QPoint> DBusConfiguration::layout() {
        return QMap<int, QPoint>();
    }

    void DBusConfiguration::activate() {
        m_parent->interface()->activate(m_name);
    }

    int DBusConfiguration::primaryScreen() {
        return m_parent->interface()->primaryScreen(m_name);
    }



    DBusConfigurations::DBusConfigurations(QObject * parent)
            : Configurations(parent)
    {
        m_interface = new org::kde::Kephal::Configurations(
            "org.kde.Kephal",
            "/modules/kephal/Configurations",
            QDBusConnection::sessionBus(),
            this);

        if (! m_interface->isValid()) {
            m_valid = false;
            return;
        }

        m_valid = true;

        const QStringList names = m_interface->configurations();
        foreach (const QString& name, names) {
            m_configs.insert(name, new DBusConfiguration(this, name));
        }

        connect(m_interface, SIGNAL(configurationActivated(QString)), this, SLOT(configurationActivatedSlot(QString)));
        connect(m_interface, SIGNAL(confirmed()), this, SIGNAL(confirmed()));
        connect(m_interface, SIGNAL(reverted()), this, SIGNAL(reverted()));
        connect(m_interface, SIGNAL(confirmTimeout(int)), this, SIGNAL(confirmTimeout(int)));
    }


    QMap<QString, Configuration *> DBusConfigurations::configurations() {
        QMap<QString, Configuration *> result;
        for (QMap<QString, DBusConfiguration *>::const_iterator i = m_configs.constBegin(); i != m_configs.constEnd(); ++i) {
            result.insert(i.key(), i.value());
        }
        return result;
    }

    Configuration * DBusConfigurations::activeConfiguration() {
        QString name = m_interface->activeConfiguration();
        if ((! name.isEmpty()) && m_configs.contains(name)) {
            return m_configs[name];
        }
        return 0;
    }

    QList<Configuration *> DBusConfigurations::alternateConfigurations() {
        const QStringList names = m_interface->alternateConfigurations();
        QList<Configuration *> result;
        foreach (const QString& name, names) {
            if (m_configs.contains(name)) {
                result << m_configs[name];
            }
        }
        return result;
    }

    QList<QPoint> DBusConfigurations::possiblePositions(const Output * output) {
        QList<QPoint> result;
        int num = m_interface->numAvailablePositions(output->id());
        for (int i = 0; i < num; ++i) {
            result << m_interface->availablePosition(output->id(), i);
        }
        return result;
    }

    bool DBusConfigurations::move(Output * output, const QPoint & position) {
        return m_interface->move(output->id(), position);
    }

    bool DBusConfigurations::resize(Output * output, const QSize & size) {
        return m_interface->resize(output->id(), size);
    }

    bool DBusConfigurations::rotate(Output * output, Rotation rotation) {
        return m_interface->rotate(output->id(), rotation);
    }

    bool DBusConfigurations::reflectX(Output * output, bool reflect) {
        return m_interface->reflectX(output->id(), reflect);
    }

    bool DBusConfigurations::reflectY(Output * output, bool reflect) {
        return m_interface->reflectY(output->id(), reflect);
    }

    bool DBusConfigurations::changeRate(Output * output, float rate) {
        return m_interface->changeRate(output->id(), rate);
    }

    bool DBusConfigurations::isValid() {
        return m_valid;
    }

    org::kde::Kephal::Configurations * DBusConfigurations::interface() {
        return m_interface;
    }

    int DBusConfigurations::screen(Output * output) {
        return m_interface->screen(output->id());
    }

    void DBusConfigurations::applyOutputSettings() {
    }

    void DBusConfigurations::setPolling(bool polling) {
        m_interface->setPolling(polling);
    }

    bool DBusConfigurations::polling() {
        return m_interface->polling();
    }

    void DBusConfigurations::configurationActivatedSlot(QString name) {
        if ((! name.isEmpty()) && m_configs.contains(name)) {
            emit configurationActivated(m_configs[name]);
        }
    }

    void DBusConfigurations::confirm() {
        m_interface->confirm();
    }

    void DBusConfigurations::revert() {
        m_interface->revert();
    }

}
