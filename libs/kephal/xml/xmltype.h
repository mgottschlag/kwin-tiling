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


#ifndef KEPHAL_XMLTYPE_H
#define KEPHAL_XMLTYPE_H

#include <QObject>
#include <QDomNode>
#include <QMap>
#include <QFile>
#include <QDebug>


namespace kephal {

    class XMLNodeHandler;
    

    class XMLType : public QObject {
        Q_OBJECT
    };
    
    
    class XMLFactory {
        public:
            XMLFactory();
            ~XMLFactory();
            
            QDomNode save(XMLType * data, QDomDocument doc, QString name);
            XMLType * load(QDomNode node);
            
        protected:
            void element(QString name, XMLNodeHandler * element);
            void attribute(QString name, XMLNodeHandler * attribute);
            
            virtual XMLType * newInstance() = 0;
            virtual void schema() = 0;
            
        private:
            QMap<QString, XMLNodeHandler *> m_elements;
            QMap<QString, XMLNodeHandler *> m_attributes;
            
            bool m_schema;
    };
    
    class XMLRootFactory : public XMLFactory {
        public:
            XMLRootFactory(QString name);
            
            bool save(XMLType * data, QString fileName);
            XMLType * load(QString fileName);
            
        private:
            QString m_name;
    };
    
}


#endif // KEPHAL_XMLTYPE_H

