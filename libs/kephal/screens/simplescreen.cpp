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


#include "simplescreen.h"


namespace kephal {

    SimpleScreen::SimpleScreen(int id, QSize size, QPoint position, bool privacy, bool primary)
        : Screen()
    {
        m_id = id;
        m_size = size;
        m_position = position;
        m_privacy = privacy;
        m_primary = primary;
    }
    
    
    int SimpleScreen::id()
    {
        return m_id;
    }

    QSize SimpleScreen::size() {
        return m_size;
    }
    
    void SimpleScreen::setSize(QSize size) {
        emit sizeChanged(this, m_size, size);
    }
    
    QPoint SimpleScreen::position() {
        return m_position;
    }

    bool SimpleScreen::isPrivacyMode()
    {
        return m_privacy;
    }
    
    void SimpleScreen::setPrivacyMode(bool privacy)
    {
        emit privacyModeChanged(this, m_privacy);
    }
    
    bool SimpleScreen::isPrimary()
    {
        return m_primary;
    }
    
    void SimpleScreen::setAsPrimary() {
        emit selectedAsPrimary(this);
    }
    
    void SimpleScreen::_setSize(QSize size) {
        m_size = size;
    }
    
    void SimpleScreen::_setPosition(QPoint position)
    {
        m_position = position;
    }
    
    void SimpleScreen::_setPrimary(bool primary) {
        m_primary = primary;
    }
    
}
