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


#ifndef KEPHAL_DBUSCONFIGURATIONS_H
#define KEPHAL_DBUSCONFIGURATIONS_H

#include <QPoint>
#include "configurations.h"
#include "configurations_interface.h"


namespace Kephal {

    class DBusConfigurations;

    /**
     * Client side representation of a Configuration.
     * Basic implementation of the Configuration interface for DBusConfigurations to return.
     * Most methods are implemented by calling back through to DBusConfigurations.
     */
    class DBusConfiguration : public Configuration {
        Q_OBJECT
        public:
            DBusConfiguration(DBusConfigurations * parent, QString name);

            /**
             * @reimp Kephal::Configuration
             */
            QString name() const;
            /**
             * @reimp Kephal::Configuration
             */
            bool isModifiable() const;
            /**
             * @reimp Kephal::Configuration
             */
            bool isActivated() const;
            /**
             * @reimp Kephal::Configuration
             */
            QMap<int, QPoint> layout() const;
            /**
             * @reimp Kephal::Configuration
             */
            int primaryScreen() const;

        public Q_SLOTS:
            void activate();

        private:
            QString m_name;
            DBusConfigurations * m_parent;
    };


    /**
     * DBUS stub to remote configurations provided by Kephal daemon
     */
    class DBusConfigurations : public Configurations {
        Q_OBJECT
        public:
            DBusConfigurations(QObject * parent);

            /**
             * @reimp Kephal::Configurations
             */
            QMap<QString, Configuration *> configurations();
            /**
             * @reimp Kephal::Configurations
             */
            Configuration * activeConfiguration();
            /**
             * @reimp Kephal::Configurations
             */
            QList<Configuration *> alternateConfigurations();
            /**
             * @reimp Kephal::Configurations
             */
            QList<QPoint> possiblePositions(const Output * output);
            /**
             * @reimp Kephal::Configurations
             */
            bool move(Output * output, const QPoint & position);
            /**
             * @reimp Kephal::Configurations
             */
            bool resize(Output * output, const QSize & size);
            /**
             * @reimp Kephal::Configurations
             */
            bool rotate(Output * output, Rotation rotation);
            /**
             * @reimp Kephal::Configurations
             */
            bool reflectX(Output * output, bool reflect);
            /**
             * @reimp Kephal::Configurations
             */
            bool reflectY(Output * output, bool reflect);
            /**
             * @reimp Kephal::Configurations
             */
            bool changeRate(Output * output, float rate);
            /**
             * @reimp Kephal::Configurations
             */
            int screen(Output * output);
            void applyOutputSettings();
            void setPolling(bool polling);
            bool polling() const;
            void confirm();
            void revert();

            bool isValid();
            org::kde::Kephal::Configurations * interface();

        private Q_SLOTS:
            void configurationActivatedSlot(QString name);

        private:
            org::kde::Kephal::Configurations * m_interface;
            bool m_valid;
            QMap<QString, DBusConfiguration *> m_configs;
    };

}


#endif // KEPHAL_DBUSCONFIGURATIONS_H

