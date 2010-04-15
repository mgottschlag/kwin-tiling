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


#ifndef KEPHAL_XMLCONFIGURATIONS_H
#define KEPHAL_XMLCONFIGURATIONS_H

#include "configurations.h"

#include "backendconfigurations.h"
#include "externalconfiguration.h"

class QTimer;

namespace Kephal {

    class XMLConfigurations;
    class ConfigurationsXML;
    class ConfigurationXML;
    class OutputsXML;
    class OutputXML;

    class XMLConfiguration;

    class XMLConfigurations : public BackendConfigurations {
        Q_OBJECT
        public:
            XMLConfigurations(QObject * parent);

            QMap<QString, Configuration *> configurations();
            /**
             * Calculates the most appropriate configuration for the current hardware configuration
             * This Configuration can then be activate()d to set up the screens as configured
             * Changes state so can't be const
             */
            Configuration * findConfiguration();
            Configuration * activeConfiguration();
            QList<Configuration *> alternateConfigurations();
            QList<QPoint> possiblePositions(Output * output);

//X             bool move(Output * output, const QPoint & position);
//X             bool resize(Output * output, const QSize & size);
//X             bool rotate(Output * output, Rotation rotation);
//X             bool changeRate(Output * output, float rate);
//X             bool reflectX(Output * output, bool reflect);
//X             bool reflectY(Output * output, bool reflect);

            int screen(Output * output);
            void applyOutputSettings();
            void setPolling(bool polling);
            bool polling() const;
            void confirm();
            void revert();

        private Q_SLOTS:
            void confirmTimerTimeout();
            bool activate(XMLConfiguration * configuration);
            void activateExternal();

        private:
            void init();
            /** populates m_currentOutputs using first findKnownOutputs then falls back to
             * findBestOutputs
             */
            void findOutputs();
            /**
             * looks for an exact match between configured sets of outputs and the current sets of
             * outputs.
             */
            OutputsXML * findKnownOutputs();
            /**
             * looks for the closest match out of configured outputs and current outputs, using a
             * scoring mechanism
             * The default hardcoded outputs should be sufficiently broad to always assure
             * some match.
             */
            OutputsXML * findBestOutputs();
            qreal match(QString known, QString current);
            qreal match(int known, int current);
            QMap<int, int> matchLayouts(const QMap<int, QPoint> & currentLayout, const QMap<int, QPoint> & layout) const;
            QMap<int, QRect> calcMatchingLayout(const QMap<int, QPoint> & currentLayout, XMLConfiguration * configuration, QMap<int, QPoint> layout, Output * output = 0, int * outputScreen = 0);
            void translateToOther(QMap<int, QRect> & layout, Output * base, QMap<int, int> match = (QMap<int, int>()));
            QList<XMLConfiguration *> equivalentConfigurations(int numScreens);
            QMap<XMLConfiguration *, QPoint> equivalentConfigurationsPositions(Output * output);
            QMap<XMLConfiguration *, QPoint> simpleConfigurationsPositions(Output * output, bool sameCount);
            QMap<XMLConfiguration *, QPoint> sameConfigurationsPositions(Output * output, bool sameCount);
            QMap<XMLConfiguration *, QMap<int, QPoint> > matchingConfigurationsLayouts(const QMap<int, QPoint> & currentLayout, int removedOutputs);
            XMLConfiguration * simpleConfiguration(int numScreens);
            void saveXml();
            void loadXml();
            /**
             * Start the layout activation process.
             *
             * @param layout absolute layout geometries
             * @param outputScreens current output->screen id mapping
             */
            bool activateLayout(const QMap<int, QRect> & layout, const QMap<Output *, int> & outputScreens);
            /**
             * Second stage of activation applying the outputs' current sizes to stored
             * configuration
             *
             * @param layout absolute layout geometries
             * @param outputScreens current output->screen id mapping
             * @param outputSizes current outputs' sizes
             */
            bool activateLayout(const QMap<int, QRect> & layout, const QMap<Output *, int> & outputScreens, const QMap<Output *, QSize> & outputSizes);
            /**
             * Obtain the current mapping from Outputs to Screens
             */
            QMap<Output *, int> currentOutputScreens();
            void matchOutputScreens(const QMap<int, QPoint> & layout);
            OutputXML * outputXml(const QString & id);
            /**
             * Helper method for resize and rotate dbus api methods;
             * adapts the layout when an output is resized or rotated.
             */
            QMap<int, QRect> resizeLayout(Output * output, const QSize & size, QMap<Output *, int> & outputScreens, QMap<Output *, QSize> & outputSizes);

            void requireConfirm();

            QMap<QString, XMLConfiguration *> m_configurations;
            XMLConfiguration * m_activeConfiguration;
            XMLConfiguration * m_markedConfiguration;
            ExternalConfiguration * m_externalConfiguration;
            ConfigurationsXML * m_configXml;
            QString m_configPath;
            OutputsXML * m_currentOutputs;
            bool m_currentOutputsKnown;
            QTimer * m_confirmTimer;
            int m_confirmLeft;
            bool m_awaitingConfirm;
    };

}


#endif // KEPHAL_XMLCONFIGURATIONS_H
