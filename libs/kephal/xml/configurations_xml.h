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
        public:
            int id() { return _id; }
            void setId(int id) { _id = id; }
            bool privacy() { return _privacy; }
            void setPrivacy(bool b) { _privacy = b; }
            
        private:
            int _id;
            bool _privacy;
    };

    class ConfigurationXML : public XMLType {
        public:
            QString name();
            void setName(QString name);
            QList<ScreenXML *> * screens();
            
        private:
            QString _name;
            QList<ScreenXML *> _screens;
    };
    
    
    
    class ConfigurationsXML : public XMLType {
        public:
            QList<ConfigurationXML *> * configurations();
            
        private:
            QList<ConfigurationXML *> _configurations;
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

