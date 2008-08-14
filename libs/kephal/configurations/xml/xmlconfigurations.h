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

#include "../configurations.h"


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
            bool modifiable();
            bool isActivated();
            void activate();
            QMap<int, QPoint> layout();
            //QMap<int, QRect> realLayout(const QMap<Output *, int> & outputScreens);
            
            ConfigurationXML * configuration();
            
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
            QList<QPoint> possiblePositions(Output *);
            
        public Q_SLOTS:
            void activate(XMLConfiguration * configuration);
            
        private:
            void init();
            void findOutputs();
            OutputsXML * findKnownOutputs();
            OutputsXML * findBestOutputs();
            qreal match(QString known, QString current);
            qreal match(int known, int current);
            QMap<int, int> matchLayouts(QMap<int, QPoint> currentLayout, QMap<int, QPoint> layout);
            QMap<int, QRect> calcMatchingLayout(QMap<int, QPoint> currentLayout, XMLConfiguration * configuration, QMap<int, QPoint> layout, Output * output = 0, int * outputScreen = 0);
            /*QMap<int, QRect> calcLayout(XMLConfiguration * configuration);
            QMap<int, QRect> calcLayout(XMLConfiguration * configuration, QMap<OutputXML *, int> outputs);
            QMap<int, QRect> calcLayout(ConfigurationXML * configuration, QMap<int, QPoint> simpleLayout, QMap<OutputXML *, int> outputs);*/
            //QMap<int, QPoint> calcSimpleLayout(XMLConfiguration * configuration);
            //QMap<int, QPoint> calcSimpleLayout(ConfigurationXML * configuration);
            QList<XMLConfiguration *> equivalentConfigurations(int numScreens);
            QMap<XMLConfiguration *, QPoint> equivalentConfigurationsPositions(Output * output);
            QMap<XMLConfiguration *, QMap<int, QPoint> > matchingConfigurationsLayouts(QMap<int, QPoint> currentLayout, int removedOutputs);
            //void simpleToReal(QMap<int, QPoint> & simpleLayout, const QMap<int, QSize> & screenSizes, const int & index, QMap<int, QRect> & screens);
            
            QMap<QString, XMLConfiguration *> m_configurations;
            XMLConfiguration * m_activeConfiguration;
            ConfigurationsXML * m_configXml;
            QString m_configPath;
            OutputsXML * m_currentOutputs;
            bool m_currentOutputsKnown;
    };
    
}


#endif // KEPHAL_XMLCONFIGURATIONS_H
