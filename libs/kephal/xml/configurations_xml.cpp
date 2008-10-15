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


namespace Kephal {

    class ScreenXMLFactory : public XMLFactory {
        protected:
            virtual XMLType * newInstance() { return new ScreenXML(); }
            virtual void schema() {
                INT_ATTRIBUTE("id", ScreenXML, id, setId);
                BOOL_ELEMENT("privacy", ScreenXML, privacy, setPrivacy);
                INT_ELEMENT("right-of", ScreenXML, rightOf, setRightOf);
                INT_ELEMENT("bottom-of", ScreenXML, bottomOf, setBottomOf);
            }
    };

    
    
    ConfigurationXML::ConfigurationXML()
        : m_modifiable(true),
        m_primaryScreen(0)
    { }
    
    QString ConfigurationXML::name() { return m_name; }
    void ConfigurationXML::setName(QString name) { m_name = name; }
    bool ConfigurationXML::modifiable() { return m_modifiable; }
    void ConfigurationXML::setModifiable(bool modifiable) { m_modifiable = modifiable; }
    QList<ScreenXML *> & ConfigurationXML::screens() { return m_screens; }
    
    
    class ConfigurationXMLFactory : public XMLFactory {
        protected:
            virtual XMLType * newInstance() { return new ConfigurationXML(); }
            virtual void schema() {
                STRING_ATTRIBUTE("name", ConfigurationXML, name, setName);
                INT_ATTRIBUTE("primary", ConfigurationXML, primaryScreen, setPrimaryScreen);
                BOOL_ATTRIBUTE("modifiable", ConfigurationXML, modifiable, setModifiable);
                COMPLEX_ELEMENT_LIST("screen", ConfigurationXML, screens, new ScreenXMLFactory(), ScreenXML);
            }
    };

    
    class OutputXMLFactory : public XMLFactory {
        protected:
            virtual XMLType * newInstance() { return new OutputXML(); }
            virtual void schema() {
                STRING_ATTRIBUTE("name", OutputXML, name, setName);
                INT_ATTRIBUTE("screen", OutputXML, screen, setScreen);
                STRING_ELEMENT("vendor", OutputXML, vendor, setVendor);
                INT_ELEMENT("product", OutputXML, product, setProduct);
                UINT_ELEMENT("serial", OutputXML, serial, setSerial);
                INT_ELEMENT("width", OutputXML, width, setWidth);
                INT_ELEMENT("height", OutputXML, height, setHeight);
                INT_ELEMENT("rotation", OutputXML, rotation, setRotation);
                BOOL_ELEMENT("reflect-x", OutputXML, reflectX, setReflectX);
                BOOL_ELEMENT("reflect-y", OutputXML, reflectY, setReflectY);
                DOUBLE_ELEMENT("refresh-rate", OutputXML, rate, setRate);
            }
    };
    
    
    class OutputsXMLFactory : public XMLFactory {
        protected:
            virtual XMLType * newInstance() { return new OutputsXML(); }
            virtual void schema() {
                STRING_ATTRIBUTE("configuration", OutputsXML, configuration, setConfiguration);
                COMPLEX_ELEMENT_LIST("output", OutputsXML, outputs, new OutputXMLFactory(), OutputXML);
            }
    };
    
    
    QList<ConfigurationXML *> & ConfigurationsXML::configurations() { return m_configurations; }
    QList<OutputsXML *> & ConfigurationsXML::outputs() { return m_outputs; }
    
    ConfigurationsXMLFactory::ConfigurationsXMLFactory() : XMLRootFactory("configurations") {
    }
    
    XMLType * ConfigurationsXMLFactory::newInstance() {
        return new ConfigurationsXML();
    }
    
    void ConfigurationsXMLFactory::schema() {
        BOOL_ELEMENT("polling", ConfigurationsXML, polling, setPolling);
        COMPLEX_ELEMENT_LIST("configuration", ConfigurationsXML, configurations, new ConfigurationXMLFactory(), ConfigurationXML);
        COMPLEX_ELEMENT_LIST("outputs", ConfigurationsXML, outputs, new OutputsXMLFactory(), OutputsXML);
    }
    
}

