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


#ifndef KEPHAL_CONFIGURATIONSXML_H
#define KEPHAL_CONFIGURATIONSXML_H

#include "xmltype.h"
#include "kephal.h"
#include "backend.h"


namespace Kephal {

    class ScreenXML : public XMLType {
        Q_OBJECT
        public:
            ScreenXML(QObject * parent = 0);
            ~ScreenXML();


        PROPERTY(int, id, setId)
        PROPERTY(int, rightOf, setRightOf)
        PROPERTY(int, bottomOf, setBottomOf)
        PROPERTY(bool, privacy, setPrivacy)

    };



    class ConfigurationXML : public XMLType {
        Q_OBJECT
        public:
            ConfigurationXML(QObject * parent = 0);
            ~ConfigurationXML();

            QList<ScreenXML *> & screens();


        PROPERTY(QString, name, setName)
        PROPERTY(bool, modifiable, setModifiable)
        PROPERTY(int, primaryScreen, setPrimaryScreen)

        private:
            QList<ScreenXML *> m_screens;
    };



    class OutputXML : public XMLType {
        Q_OBJECT

        PROPERTY(QString, name, setName)
        PROPERTY(int, screen, setScreen)
        PROPERTY(QString, vendor, setVendor)
        PROPERTY(int, product, setProduct)
        PROPERTY(unsigned int, serial, setSerial)
        PROPERTY(int, width, setWidth)
        PROPERTY(int, height, setHeight)
        PROPERTY(int, rotation, setRotation)
        PROPERTY(bool, reflectX, setReflectX)
        PROPERTY(bool, reflectY, setReflectY)
        PROPERTY(double, rate, setRate)
        PROPERTY(QString, actualOutput, setActualOutput)

        public:
        OutputXML(QObject * parent = 0);
        ~OutputXML();

    };



    class OutputsXML : public XMLType {
        Q_OBJECT
        public:
            OutputsXML(QObject * parent = 0);
            ~OutputsXML();

        PROPERTY(QString, configuration, setConfiguration)

        public:
            QList<OutputXML *> & outputs() { return m_outputs; }
        private:
            QList<OutputXML *> m_outputs;
    };



    class ConfigurationsXML : public XMLType {
        Q_OBJECT

        PROPERTY(bool, polling, setPolling)

        public:
            ConfigurationsXML(QObject * parent = 0);
            virtual ~ConfigurationsXML();

            QList<ConfigurationXML *> & configurations();
            QList<OutputsXML *> & outputs();

        private:
            QList<ConfigurationXML *> m_configurations;
            QList<OutputsXML *> m_outputs;
    };

    class ConfigurationsXMLFactory : public XMLRootFactory {
        public:
            ConfigurationsXMLFactory();
            ~ConfigurationsXMLFactory();

        protected:
            virtual XMLType * newInstance();
            virtual void schema();
    };

}

#endif // KEPHAL_CONFIGURATIONSXML_H

