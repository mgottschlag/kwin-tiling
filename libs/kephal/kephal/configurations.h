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


#ifndef KEPHAL_CONFIGURATIONS_H
#define KEPHAL_CONFIGURATIONS_H

#include <QMap>
#include <QObject>
#include <QRect>

#include "kephal.h"

namespace Kephal {

    class Output;


    /**
     * A Configuration allows to change settings that
     * control which Output belongs to which Screen or
     * is inactive.
     */
    class Configuration : public QObject {
        Q_OBJECT
        public:
            Configuration(QObject * parent);
            virtual ~Configuration() {}

            /**
             * The name of the Configuration.
             * This name uniquely identifies the
             * Configuration.
             */
            virtual QString name() const = 0;

            /**
             * Returns whether this Configuration
             * can be modified.
             * This includes the layout and settings
             * like privacy-mode and primary Screen.
             */
            virtual bool isModifiable() const = 0;

            /**
             * Returns whether this Configuration is
             * currently active.
             * Only 1 Configuration can be active at
             * any time.
             */
            virtual bool isActivated() const = 0;

            /**
             * Returns the layout as Screens of size
             * 1x1.
             */
            virtual QMap<int, QPoint> layout() const = 0;

            /**
             * Returns the id of the primary Screen for
             * this Configuration.
             */
            virtual int primaryScreen() const = 0;

        public Q_SLOTS:
            /**
             * Activate this Configuration.
             */
            virtual void activate() = 0;
    };



    /**
     * Configurations is the entrance-point to all
     * Configuration related operations.
     * Use: Configurations::self() to obtain the currently
     * active instance.
     */
    class Configurations : public QObject {
        Q_OBJECT
        public:
            /**
             * Returns the currently active
             * instance.
             */
            static Configurations * self();

            Configurations(QObject * parent);
            virtual ~Configurations();

            /**
             * Returns a list of all known Configurations.
             */
            virtual QMap<QString, Configuration *> configurations() = 0;

            /**
             * Returns the currently active Configuration.
             */
            virtual Configuration * activeConfiguration() = 0;

            /**
             * Returns a list of all alternate Configuratios
             * for the currently connected Outputs.
             */
            virtual QList<Configuration *> alternateConfigurations() = 0;

            /**
             * Returns the list of all positions in pixels
             * for the Output output.
             * These are the only positions that can be
             * passed to move().
             */
            virtual QList<QPoint> possiblePositions(Output * output) = 0;

            /**
             * Find a Configuration by its name.
             * This returns 0 if the name is not known.
             */
            virtual Configuration * configuration(QString name);

            /**
             * Return the id of the Screen this Output should
             * belong to.
             */
            virtual int screen(Output * output) = 0;

            virtual void setPolling(bool polling) = 0;
            virtual bool polling() const = 0;

            virtual void confirm() = 0;
            virtual void revert() = 0;

            //virtual StatusMessage * status() = 0;

            static void translateOrigin(QMap<int, QPoint> & layout);
            static void translateOrigin(QMap<int, QPoint> & layout, QPoint origin);
            static void translateOrigin(QMap<int, QRect> & layout);
            static void translateOrigin(QMap<int, QRect> & layout, QPoint origin);

        Q_SIGNALS:
            /**
             * This signal is emitted when the active
             * Configuration is changed.
             */
            void configurationActivated(Kephal::Configuration * configuration);

            void pollingActivated();
            void pollingDeactivated();

            void confirmTimeout(int seconds);
            void confirmed();
            void reverted();

        protected:
            static Configurations * s_instance;
    };

}


#endif // KEPHAL_CONFIGURATIONS_H

