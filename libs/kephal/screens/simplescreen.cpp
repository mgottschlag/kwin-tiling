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

    SimpleScreen::SimpleScreen(int id, QSize resolution, QPoint position, bool privacy, bool primary)
        : Screen()
    {
        _id = id;
        _resolution = resolution;
        _position = position;
        _privacy = privacy;
        _primary = primary;
    }
    
    
    int SimpleScreen::getId()
    {
        return _id;
    }

    QSize SimpleScreen::getResolution() {
        return _resolution;
    }
    
    void SimpleScreen::setResolution(QSize size) {
        /*QSize old = _resolution;
        if (old == size) {
            return;
        }
        _resolution = size;*/
        emit resolutionChanged(this, _resolution, size);
    }
    
    QPoint SimpleScreen::getPosition() {
        return _position;
    }
    //QList<PositionType> getRelativePosition();

    bool SimpleScreen::isPrivacyMode()
    {
        return _privacy;
    }
    
    void SimpleScreen::setPrivacyMode(bool privacy)
    {
        /*if (_privacy == privacy) {
            return;
        }
        
        _privacy = privacy;*/
        emit privacyModeChanged(this, _privacy);
    }
    
    bool SimpleScreen::isPrimary()
    {
        return _primary;
    }
    
    void SimpleScreen::setAsPrimary() {
        /*if (_primary) {
            return;
        }
        _primary = true;*/
        emit selectedAsPrimary(this);
    }
    
    void SimpleScreen::_setResolution(QSize size) {
        _resolution = size;
    }
    
    void SimpleScreen::_setPosition(QPoint position)
    {
        _position = position;
    }
    
    void SimpleScreen::_setPrimary(bool primary) {
        _primary = primary;
    }
    
}
