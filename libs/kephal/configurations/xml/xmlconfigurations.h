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

#include <QRect>

#include "kephal/configurations.h"


namespace kephal {

    class XMLConfigurations;
    class ConfigurationsXML;
    class ConfigurationXML;
    class OutputsXML;
    class OutputXML;
    
    

    class XMLConfiguration : public Configuration {
        Q_OBJECT
        public:
            XMLConfiguration(XMLConfigurations * parent, ConfigurationXML * configuration);
            
            QString name();
            bool isModifiable();
            bool isActivated();
            void activate();
            QMap<int, QPoint> layout();
            int primaryScreen();
            
            ConfigurationXML * configuration();
            void setLayout(const QMap<int, QPoint> & layout);
            
        Q_SIGNALS:
            void activate(XMLConfiguration * configuration);
            
        private:
            ConfigurationXML * m_configuration;
            XMLConfigurations * m_parent;
            QMap<int, QPoint> m_layout;
    };
    
    
    class XMLConfigurations : public Configurations {
        Q_OBJECT
        public:
            XMLConfigurations(QObject * parent);
            
            QMap<QString, Configuration *> configurations();
            Configuration * findConfiguration();
            Configuration * activeConfiguration();
            QList<Configuration *> alternateConfigurations();
            QList<QPoint> possiblePositions(Output * output);
            void move(Output * output, const QPoint & position);
            void resize(Output * output, const QSize & size);
            void rotate(Output * output, Rotation rotation);
            void changeRate(Output * output, float rate);
            void reflectX(Output * output, bool reflect);
            void reflectY(Output * output, bool reflect);
            int screen(Output * output);
            void applyOutputSettings();
            void setPolling(bool polling);
            bool polling();
            
        public Q_SLOTS:
            void activate(XMLConfiguration * configuration);
            
        private:
            void init();
            void findOutputs();
            OutputsXML * findKnownOutputs();
            OutputsXML * findBestOutputs();
            qreal match(QString known, QString current);
            qreal match(int known, int current);
            QMap<int, int> matchLayouts(const QMap<int, QPoint> & currentLayout, QMap<int, QPoint> layout);
            QMap<int, QRect> calcMatchingLayout(const QMap<int, QPoint> & currentLayout, XMLConfiguration * configuration, QMap<int, QPoint> layout, Output * output = 0, int * outputScreen = 0);
            void translateToOther(QMap<int, QRect> & layout, Output * base, QMap<int, int> match = (QMap<int, int>()));
            QList<XMLConfiguration *> equivalentConfigurations(int numScreens);
            QMap<XMLConfiguration *, QPoint> equivalentConfigurationsPositions(Output * output);
            QMap<XMLConfiguration *, QPoint> simpleConfigurationsPositions(Output * output, bool sameCount);
            QMap<XMLConfiguration *, QPoint> sameConfigurationsPositions(Output * output, bool sameCount);
            QMap<XMLConfiguration *, QMap<int, QPoint> > matchingConfigurationsLayouts(const QMap<int, QPoint> & currentLayout, int removedOutputs);
            XMLConfiguration * simpleConfiguration(int numScreens);
            void saveXml();
            bool activateLayout(const QMap<int, QRect> & layout, const QMap<Output *, int> & outputScreens);
            bool activateLayout(const QMap<int, QRect> & layout, const QMap<Output *, int> & outputScreens, const QMap<Output *, QSize> & outputSizes);
            QMap<Output *, int> currentOutputScreens();
            void matchOutputScreens(const QMap<int, QPoint> & layout);
            OutputXML * outputXml(const QString & id);
            QMap<int, QRect> resizeLayout(Output * output, const QSize & size, QMap<Output *, int> & outputScreens, QMap<Output *, QSize> & outputSizes);
            
            QMap<QString, XMLConfiguration *> m_configurations;
            XMLConfiguration * m_activeConfiguration;
            ConfigurationsXML * m_configXml;
            QString m_configPath;
            OutputsXML * m_currentOutputs;
            bool m_currentOutputsKnown;
    };
    
}


#endif // KEPHAL_XMLCONFIGURATIONS_H
