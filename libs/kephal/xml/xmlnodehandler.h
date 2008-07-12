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


#ifndef KEPHAL_XMLNODES_H
#define KEPHAL_XMLNODES_H


#include "xmltype.h"

#include <QDomNode>


namespace kephal {

    class XMLType;
    class XMLFactory;
    

    class XMLNodeHandler {
        public:
            virtual void beginSave(XMLType * element) = 0;
            virtual void beginLoad(XMLType * element) = 0;
            virtual bool hasMore(XMLType * element) = 0;
            virtual void setNode(XMLType * element, QDomNode node) = 0;
            virtual QDomNode node(XMLType * element, QDomDocument doc, QString name) = 0;
            virtual QString str(XMLType * element) = 0;
    };
    
    
    template <class ElementType, typename SimpleType>
    class XMLSimpleNodeHandler : public XMLNodeHandler {
        public:
            typedef void (ElementType::*Setter)(SimpleType);
            typedef SimpleType (ElementType::*Getter)();
            
            XMLSimpleNodeHandler(Getter getter, Setter setter);
            
            virtual void beginSave(XMLType * element);
            virtual void beginLoad(XMLType * element);
            virtual bool hasMore(XMLType * element);
            virtual void setNode(XMLType * element, QDomNode node);
            virtual QDomNode node(XMLType * element, QDomDocument doc, QString name);
            virtual QString str(XMLType * element);
        
        protected:
            virtual SimpleType toValue(QString str) = 0;
            virtual QString toString(SimpleType value) = 0;
            
        private:
            Getter _getter;
            Setter _setter;
            bool _saved;
    };
    
    
    template <class ElementType>
    class XMLStringNodeHandler : public XMLSimpleNodeHandler<ElementType, QString> {
        public:
            typedef void (ElementType::*Setter)(QString);
            typedef QString (ElementType::*Getter)();
            
            XMLStringNodeHandler(Getter getter, Setter setter);
        
        protected:
            virtual QString toValue(QString str);
            virtual QString toString(QString value);
    };
    
    
    template <class ElementType>
    class XMLIntNodeHandler : public XMLSimpleNodeHandler<ElementType, int> {
        public:
            typedef void (ElementType::*Setter)(int);
            typedef int (ElementType::*Getter)();
            
            XMLIntNodeHandler(Getter getter, Setter setter);
        
        protected:
            virtual int toValue(QString str);
            virtual QString toString(int value);
    };
    
    
    template <class ElementType>
    class XMLBoolNodeHandler : public XMLSimpleNodeHandler<ElementType, bool> {
        public:
            typedef void (ElementType::*Setter)(bool);
            typedef bool (ElementType::*Getter)();
            
            XMLBoolNodeHandler(Getter getter, Setter setter);
        
        protected:
            virtual bool toValue(QString str);
            virtual QString toString(bool value);
    };
    
    
    template <class ElementType, class ComplexType>
    class XMLComplexNodeHandler : public XMLNodeHandler {
        public:
            typedef void (ElementType::*Setter)(ComplexType *);
            
            XMLComplexNodeHandler(XMLFactory * factory, Setter setter);
            
            virtual void beginSave(XMLType * element);
            virtual void beginLoad(XMLType * element);
            virtual bool hasMore(XMLType * element);
            virtual void setNode(XMLType * element, QDomNode node);
            virtual QDomNode node(XMLType * element, QDomDocument doc, QString name);
            virtual QString str(XMLType * element);
        
        private:
            XMLFactory * _factory;
            Setter _setter;
            bool _saved;
    };
    
    
    template <class ElementType, class ComplexType>
    class XMLComplexListNodeHandler : public XMLNodeHandler {
        public:
            typedef QList<ComplexType *> * (ElementType::*ListGetter)();
            
            XMLComplexListNodeHandler(XMLFactory * factory, ListGetter listGetter);
            
            virtual void beginSave(XMLType * element);
            virtual void beginLoad(XMLType * element);
            virtual bool hasMore(XMLType * element);
            virtual void setNode(XMLType * element, QDomNode node);
            virtual QDomNode node(XMLType * element, QDomDocument doc, QString name);
            virtual QString str(XMLType * element);
        
        private:
            XMLFactory * _factory;
            ListGetter _listGetter;
            int _pos;
    };
    
}

#include "xmlnodehandler.h.cpp"


#define STRING_ATTRIBUTE(name, class, getter, setter) attribute(name, new XMLStringNodeHandler<class>(&class::getter, &class::setter))
#define INT_ATTRIBUTE(name, class, getter, setter) attribute(name, new XMLIntNodeHandler<class>(&class::getter, &class::setter))
#define BOOL_ATTRIBUTE(name, class, getter, setter) attribute(name, new XMLBoolNodeHandler<class>(&class::getter, &class::setter))

#define STRING_ELEMENT(name, class, getter, setter) element(name, new XMLStringNodeHandler<class>(&class::getter, &class::setter))
#define INT_ELEMENT(name, class, getter, setter) element(name, new XMLIntNodeHandler<class>(&class::getter, &class::setter))
#define BOOL_ELEMENT(name, class, getter, setter) element(name, new XMLBoolNodeHandler<class>(&class::getter, &class::setter))
#define COMPLEX_ELEMENT(name, class, setter, factory, complex) element(name, new XMLComplexNodeHandler<class, complex>(factory, &class::setter))
#define COMPLEX_ELEMENT_LIST(name, class, listGetter, factory, complex) element(name, new XMLComplexListNodeHandler<class, complex>(factory, &class::listGetter))


#endif // KEPHAL_XMLNODES_H

