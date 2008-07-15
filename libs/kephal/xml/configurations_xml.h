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

#include <QString>


namespace kephal {

    class ScreenXML : public XMLType {
        Q_OBJECT
        public:
            int id() { return m_id; }
            void setId(int id) { m_id = id; }
            bool privacy() { return m_privacy; }
            void setPrivacy(bool b) { m_privacy = b; }
            
        private:
            int m_id;
            bool m_privacy;
    };

    class ConfigurationXML : public XMLType {
        Q_OBJECT
        public:
            QString name();
            void setName(QString name);
            QList<ScreenXML *> * screens();
            
        private:
            QString m_name;
            QList<ScreenXML *> m_screens;
    };
    
    
    
    class ConfigurationsXML : public XMLType {
        Q_OBJECT
        public:
            QList<ConfigurationXML *> * configurations();
            
        private:
            QList<ConfigurationXML *> m_configurations;
    };
    
    class ConfigurationsXMLFactory : public XMLRootFactory {
        public:
            ConfigurationsXMLFactory();
            
        protected:
            virtual XMLType * newInstance();
            virtual void schema();
    };

}

#endif // KEPHAL_CONFIGURATIONSXML_H

