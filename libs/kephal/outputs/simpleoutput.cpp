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


#include "simpleoutput.h"


namespace kephal {

    SimpleOutput::SimpleOutput(QObject * parent, QString id, QSize size, QPoint position, bool connected, bool activated)
        : Output(parent)
    {
        m_id = id;
        m_size = size;
        m_position = position;
        m_connected = connected;
        m_activated = activated;
    }
    
    SimpleOutput::SimpleOutput(QObject * parent)
        : Output(parent),
        m_id(""),
        m_size(0, 0),
        m_position(0, 0),
        m_connected(false),
        m_activated(false)
    {
    }
    
    
    QString SimpleOutput::id()
    {
        return m_id;
    }

    QSize SimpleOutput::size() {
        return m_size;
    }
    
    void SimpleOutput::setSize(QSize size) {
        emit sizeChangeRequested(this, m_size, size);
    }
    
    QPoint SimpleOutput::position() {
        return m_position;
    }

    void SimpleOutput::_setId(QString id) {
        m_id = id;
    }
    
    void SimpleOutput::_setSize(QSize size) {
        m_size = size;
    }
    
    void SimpleOutput::_setPosition(QPoint position) {
        m_position = position;
    }
    
    void SimpleOutput::_setConnected(bool connected) {
        m_connected = connected;
    }
    
    void SimpleOutput::_setActivated(bool activated) {
        m_activated = activated;
    }
    
    bool SimpleOutput::isConnected() {
        return m_connected;
    }
    
    bool SimpleOutput::isActivated() {
        return m_connected && m_activated;
    }
    
}
