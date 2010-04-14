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


#ifndef XML_CONFIGURATION_H
#define XML_CONFIGURATION_H

#include "backendconfigurations.h"

namespace Kephal {

    class XMLConfigurations;
    class ConfigurationsXML;
    class ConfigurationXML;

    class XMLConfiguration : public BackendConfiguration {
        Q_OBJECT
        public:
            XMLConfiguration(XMLConfigurations * parent, ConfigurationXML * configuration);

            QString name() const;
            bool isModifiable() const;
            bool isActivated() const;
            void activate();
            QMap<int, QPoint> layout() const;
            int primaryScreen() const;

            ConfigurationXML * configuration() const;
            void setLayout(const QMap<int, QPoint> & layout);

        Q_SIGNALS:
            void configurationActivated(XMLConfiguration * configuration);

        private:
            ConfigurationXML * m_configuration;
            XMLConfigurations * m_parent;
            QMap<int, QPoint> m_layout;
    };
}
#endif // XMLCONFIGURATION_H
