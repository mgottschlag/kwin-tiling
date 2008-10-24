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


#include "configurations.h"

#include <QDebug>
#include <QRegExp>

#ifdef CONFIGURATIONS_FACTORY
void CONFIGURATIONS_FACTORY();
#endif

#include "kephal/outputs.h"
#include "kephal/screens.h"
#include "kephal/backend.h"


namespace Kephal {

    Configurations * Configurations::self() {
#ifdef CONFIGURATIONS_FACTORY
        if (Configurations::m_instance == 0) {
            CONFIGURATIONS_FACTORY();
        }
#endif
        return Configurations::m_instance;
    }
    
    Configurations::Configurations(QObject * parent)
            : QObject(parent)
    {
        Configurations::m_instance = this;
    }
    
    Configurations * Configurations::m_instance = 0;
    
    Configuration * Configurations::configuration(QString name) {
        foreach (Configuration * config, configurations()) {
            if (config->name() == name) {
                return config;
            }
        }
        return 0;
    }
    
    void Configurations::translateOrigin(QMap<int, QPoint> & layout) {
        QPoint origin;
        bool first = true;
        for (QMap<int, QPoint>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (first || (i.value().x() < origin.x())) {
                origin.setX(i.value().x());
            }
            if (first || (i.value().y() < origin.y())) {
                origin.setY(i.value().y());
            }
            first = false;
        }
        translateOrigin(layout, origin);
    }
    
    void Configurations::translateOrigin(QMap<int, QPoint> & layout, QPoint origin) {
        for (QMap<int, QPoint>::iterator i = layout.begin(); i != layout.end(); ++i) {
            i.value() -= origin;
        }
    }
    
    void Configurations::translateOrigin(QMap<int, QRect> & layout) {
        QPoint origin;
        bool first = true;
        for (QMap<int, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (first || (i.value().x() < origin.x())) {
                origin.setX(i.value().x());
            }
            if (first || (i.value().y() < origin.y())) {
                origin.setY(i.value().y());
            }
            first = false;
        }
        translateOrigin(layout, origin);
    }
    
    void Configurations::translateOrigin(QMap<int, QRect> & layout, QPoint origin) {
        QPoint offset(0, 0);
        offset -= origin;
        for (QMap<int, QRect>::iterator i = layout.begin(); i != layout.end(); ++i) {
            i.value().translate(offset);
        }
    }
    
    
    
    Configuration::Configuration(QObject * parent)
            : QObject(parent)
    {
    }
    
    
    
    /*StatusMessage::StatusMessage(StatusMessage::MessageType messageType, StatusMessage::Message message, QString description, QObject * parent)
        : QObject(parent),
        m_type(messageType),
        m_message(message),
        m_description(description)
    {
    }
    
    StatusMessage::StatusMessage(StatusMessage::MessageType messageType, StatusMessage::Message message, QObject * parent)
        : QObject(parent),
        m_type(messageType),
        m_message(message),
        m_description("")
    {
    }
    
    StatusMessage::StatusMessage(QObject * parent)
        : QObject(parent),
        m_type(TypeNone),
        m_message(NoMessage),
        m_description("")
    {
    }
    
    StatusMessage::MessageType StatusMessage::type() {
        return m_type;
    }
    
    StatusMessage::Message StatusMessage::message() {
        return m_message;
    }
    
    QString StatusMessage::description() {
        return m_description;
    }
    
    QString StatusMessage::toString() {
        QString result = "";
        
        switch (m_type) {
            case TypeNone:
                return "<none>";
            case TypeInfo:
                result += "[INFO] ";
                break;
            case TypeWarning:
                result += "[WARNING] ";
                break;
            case TypeError:
                result += "[ERROR] ";
                break;
            default:
                result += "[?? (" + QString::number(m_type) + ")] ";
        }
        
        switch (m_message) {
            case InvalidConfiguration:
                result += "InvalidConfiguration: ";
                break;
            default:
                result += "Unknown (" + QString::number(m_message) + "): ";
        }
        
        return result + m_description;
    }*/
    
}

