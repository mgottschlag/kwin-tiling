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


#include "xmltype.h"
#include "xmlnodehandler.h"

#include <QFile>
#include <QDebug>


namespace kephal {

    XMLFactory::XMLFactory() {
        m_schema = false;
    }
    
    XMLFactory::~XMLFactory() {
        foreach (XMLNodeHandler * n, m_attributes.values()) {
            delete n;
        }
        foreach (XMLNodeHandler * n, m_elements.values()) {
            delete n;
        }
    }
    
    XMLType * XMLRootFactory::load(QString fileName) {
        QFile file(fileName);
        if (! file.open(QIODevice::ReadOnly)) {
            //qDebug() << "couldnt open file" << fileName;
            if (! fileName.endsWith("~")) {
                return load(fileName + "~");
            }
            return 0;
        }
        
        QDomDocument dom;
        if (! dom.setContent(&file)) {
            //qDebug() << "couldnt parse xml!!";
            file.close();
            if (! fileName.endsWith("~")) {
                return load(fileName + "~");
            }
            return 0;
        }
        file.close();
        
        QDomElement root = dom.documentElement();
        if (root.nodeName() == m_name) {
            return XMLFactory::load(root);
        } else {
            return 0;
        }
    }
    
    XMLType * XMLFactory::load(QDomNode root) {
        if (! m_schema) {
            schema();
            m_schema = true;
        }
        
        //qDebug() << "root:" << root.isElement() << root.nodeName();
        if (! root.isElement()) {
            return 0;
        }
        
        XMLType * result = newInstance();
        if (! result) {
            //qDebug() << "newInstance() returned 0";
            return 0;
        }
        
        foreach (XMLNodeHandler * n, m_attributes.values()) {
            n->beginLoad(result);
        }
        foreach (XMLNodeHandler * n, m_elements.values()) {
            n->beginLoad(result);
        }
        
        QDomNamedNodeMap attrs = root.attributes();
        for (int i = 0; i < attrs.size(); ++i) {
            QDomNode attr = attrs.item(i);
            //qDebug() << "attr:" << attr.isElement() << attr.nodeName();
            
            QString name = attr.nodeName();
            if (m_attributes.contains(name)) {
                //qDebug() << "is known attribute...";
                XMLNodeHandler * xmlNode = m_attributes.value(name);
                xmlNode->setNode(result, attr);
                //qDebug() << "value has been set!!";
            }
        }
        
        QDomNode node = root.firstChild();
        while (! node.isNull()) {
            //qDebug() << "node:" << node.isElement() << node.nodeName();
            if (! node.isElement()) {
                continue;
            }
            
            QString name = node.nodeName();
            if (m_elements.contains(name)) {
                //qDebug() << "is known element...";
                XMLNodeHandler * xmlNode = m_elements.value(name);
                xmlNode->setNode(result, node);
                //qDebug() << "value has been set!!";
            }

            node = node.nextSibling();
        }
        
        return result;
    }
    
    bool XMLRootFactory::save(XMLType * data, QString fileName) {
        QDomDocument doc;
        QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version=\"1.0\"");
        doc.appendChild(header);
        QDomNode node = XMLFactory::save(data, doc, m_name);
        if (! node.isNull()) {
            doc.appendChild(node);
        }
        QString content = doc.toString();
        
        QFile file(fileName);
        QFile backup(fileName + "~");
        if (file.exists()) {
            if (backup.exists()) {
                if (! backup.remove()) {
                    return false;
                }
            }
            if (! file.rename(backup.fileName())) {
                return false;
            }
        }
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&file);
            out << content;
            file.close();
            
            if (file.error() != QFile::NoError) {
                return false;
            }
            
            backup.remove();
            return true;
        }
        
        return false;
    }
    
    QDomNode XMLFactory::save(XMLType * data, QDomDocument doc, QString name) {
        if (! m_schema) {
            schema();
            m_schema = true;
        }
        
        QDomElement node = doc.createElement(name);
        for (QMap<QString, XMLNodeHandler *>::const_iterator i = m_attributes.constBegin(); i != m_attributes.constEnd(); ++i) {
            //qDebug() << "save attribute:" << i.key();
            QString value = i.value()->str(data);
            if (! value.isNull()) {
                node.setAttribute(i.key(), value);
            }
        }
        for (QMap<QString, XMLNodeHandler *>::const_iterator i = m_elements.constBegin(); i != m_elements.constEnd(); ++i) {
            //qDebug() << "save element:" << i.key();
            i.value()->beginSave(data);
            while (i.value()->hasMore(data)) {
                //qDebug() << "-> instance";
                QDomNode child = i.value()->node(data, doc, i.key());
                if (! child.isNull()) {
                    node.appendChild(child);
                }
            }
        }
        return node;
    }
    
    void XMLFactory::element(QString name, XMLNodeHandler * element) {
        m_elements.insert(name, element);
    }
    
    void XMLFactory::attribute(QString name, XMLNodeHandler * attribute) {
        m_attributes.insert(name, attribute);
    }
    
    XMLRootFactory::XMLRootFactory(QString name) {
        m_name = name;
    }
    
}

