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


#include "configurations_xml.h"

#include <QDebug>
#include "xmlnodehandler.h"


namespace kephal {

    class ScreenXMLFactory : public XMLFactory {
        protected:
            virtual XMLType * newInstance() { return new ScreenXML(); }
            virtual void schema() {
                INT_ATTRIBUTE("id", ScreenXML, id, setId);
                BOOL_ELEMENT("privacy", ScreenXML, privacy, setPrivacy);
            }
    };

    
    
    QString ConfigurationXML::name() { return _name; }
    void ConfigurationXML::setName(QString name) { _name = name; }
    QList<ScreenXML *> * ConfigurationXML::screens() { return & _screens; }
    
    /*XMLType * ConfigurationXMLFactory::newInstance() {
        return new ConfigurationXML();
    }
    
    void ConfigurationXMLFactory::schema() {
        STRING_ATTRIBUTE("name", ConfigurationXML, setName);
    }*/
    
    class ConfigurationXMLFactory : public XMLFactory {
        protected:
            virtual XMLType * newInstance() { return new ConfigurationXML(); }
            virtual void schema() {
                STRING_ATTRIBUTE("name", ConfigurationXML, name, setName);
                COMPLEX_ELEMENT_LIST("screen", ConfigurationXML, screens, new ScreenXMLFactory(), ScreenXML);
            }
    };

    
    
    QList<ConfigurationXML *> * ConfigurationsXML::configurations() { return & _configurations; }
    
    ConfigurationsXMLFactory::ConfigurationsXMLFactory() : XMLRootFactory("configurations") {
    }
    
    XMLType * ConfigurationsXMLFactory::newInstance() {
        return new ConfigurationsXML();
    }
    
    void ConfigurationsXMLFactory::schema() {
        COMPLEX_ELEMENT_LIST("configuration", ConfigurationsXML, configurations, new ConfigurationXMLFactory(), ConfigurationXML);
    }
    
}

