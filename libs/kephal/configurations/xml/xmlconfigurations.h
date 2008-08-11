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

#include "../configurations.h"


namespace kephal {

    class XMLConfigurations;
    class ConfigurationsXML;
    class ConfigurationXML;
    class OutputsXML;
    
    

    class XMLConfiguration : public Configuration {
        Q_OBJECT
        public:
            XMLConfiguration(XMLConfigurations * parent, ConfigurationXML * configuration);
            
            QString name();
            bool modifiable();
            bool isActivated();
            void activate();
            
            ConfigurationXML * configuration();
            
        Q_SIGNALS:
            void activate(XMLConfiguration * configuration);
            
        private:
            ConfigurationXML * m_configuration;
            XMLConfigurations * m_parent;
    };
    
    
    class XMLConfigurations : public Configurations {
        Q_OBJECT
        public:
            XMLConfigurations(QObject * parent);
            
            QMap<QString, Configuration *> configurations();
            Configuration * findConfiguration();
            Configuration * activeConfiguration();
            
        public Q_SLOTS:
            void activate(XMLConfiguration * configuration);
            
        private:
            void init();
            void findOutputs();
            OutputsXML * findKnownOutputs();
            OutputsXML * findBestOutputs();
            qreal match(QString known, QString current);
            qreal match(int known, int current);
            
            QMap<QString, XMLConfiguration *> m_configurations;
            XMLConfiguration * m_activeConfiguration;
            ConfigurationsXML * m_configXml;
            QString m_configPath;
            OutputsXML * m_currentOutputs;
            bool m_currentOutputsKnown;
    };
    
}


#endif // KEPHAL_XMLCONFIGURATIONS_H
