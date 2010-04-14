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


#ifndef KEPHAL_BACKENDCONFIGURATIONS_H
#define KEPHAL_BACKENDCONFIGURATIONS_H


#include <QMap>
#include <QPoint>
#include <QRect>

#include "configurations.h"


namespace Kephal {

    /**
     * A configuration forming part of the backend (Kephal service)
     */
    class BackendConfiguration : public Configuration {
        Q_OBJECT
        public:
            BackendConfiguration(QObject * parent);

            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             *
             * @param simpleLayout The layout as returned
             *          from layout().
             * @param outputScreens A mapping of Outputs
             *          to Screens.
             * @param outputSizes The sizes to use for the
             *          Outputs instead of the current ones.
             * WILL: used by XMLConf::resizeLayout()
             */
            QMap<int, QRect> realLayout(const QMap<int, QPoint> & simpleLayout, const QMap<Output *, int> & outputScreens, const QMap<Output *, QSize> & outputSizes);

            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             *
             * @param simpleLayout The layout as returned
             *          from layout().
             * @param outputScreens A mapping of Outputs
             *          to Screens.
             */
            QMap<int, QRect> realLayout(const QMap<int, QPoint> & simpleLayout, const QMap<Output *, int> & outputScreens);

            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             * This will calculate the layout by calling
             * layout().
             *
             * @param outputScreens A mapping of Outputs
             *          to Screens.
             * WILL used by XMLConfiguration::activate(Configuration*)
             * WILL used by XMLConfiguration::calcMatchingLayout()
             * WILL used by XMLConfiguration::simpleConfigurationsPositions()
             */
            QMap<int, QRect> realLayout(const QMap<Output *, int> & outputScreens);

            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             * This will calculate the layout by calling
             * layout() and use the Output to Screen
             * mapping as currently active if possible.
             */
            QMap<int, QRect> realLayout();

            /**
             * Returns a set of points covered in the
             * layout returned by layout().
             */
            QSet<QPoint> positions();

            /**
             * Returns the positions as in positions
             * to which the Screen can be cloned.
             */
            QSet<QPoint> clonePositions(int screen);

            /**
             * Returns the layout if the Screen screen
             * was to be cloned to any of the other
             * Screens.
             */
            QMap<int, QPoint> cloneLayout(int screen);

            /**
             * Returns the possible positions as in
             * positions() to move the Screen screen
             * to.
             */
            QSet<QPoint> possiblePositions(int screen) const;

        private:
            void simpleToReal(QMap<int, QPoint> & simpleLayout, const QMap<int, QSize> & screenSizes, int index, QMap<int, QRect> & screens) const;
            QList<QSet<QPoint> > partition(int screen) const;
            QSet<QPoint> border(QSet<QPoint> screens) const;
    };



    /**
     * A manager of configurations, on the backend
     */
    class BackendConfigurations : public Configurations {
        Q_OBJECT
        public:
            static BackendConfigurations * self();

            BackendConfigurations(QObject * parent);
            virtual ~BackendConfigurations();

            /**
             * Find the Configuration for the currently
             * connected Outputs.
             */
            virtual Configuration * findConfiguration() = 0;

            /**
             * Apply Output-specific settings such as size,
             * refresh-rate and rotation.
             */
            virtual void applyOutputSettings() = 0;

            virtual BackendConfiguration * activeBackendConfiguration();

        private:
            static BackendConfigurations * s_instance;
    };

}


#endif // KEPHAL_BACKENDCONFIGURATIONS_H

