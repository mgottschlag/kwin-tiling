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


namespace Kephal {

    SimpleScreen::SimpleScreen(int id, const QSize & size, const QPoint & position, bool privacy, QObject * parent)
        : Screen(parent)
    {
        m_id = id;
        m_size = size;
        m_position = position;
        m_privacy = privacy;
    }

    SimpleScreen::SimpleScreen(QObject * parent)
        : Screen(parent),
        m_id(-1),
        m_size(0, 0),
        m_position(0, 0),
        m_privacy(false)
    {
    }

    SimpleScreen::~SimpleScreen()
    {

    }

    int SimpleScreen::id()
    {
        return m_id;
    }

    QSize SimpleScreen::size() {
        return m_size;
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
        emit privacyModeChangeRequested(this, privacy);
    }

    void SimpleScreen::_setId(int id) {
        m_id = id;
    }

    void SimpleScreen::_setSize(const QSize & size) {
        m_size = size;
    }

    void SimpleScreen::_setPosition(const QPoint & position)
    {
        m_position = position;
    }

    void SimpleScreen::_setGeom(const QRect & geom)
    {
        _setPosition(geom.topLeft());
        _setSize(geom.size());
    }

    QList<Output *> SimpleScreen::outputs() {
        return m_outputs;
    }

    QList<Output *> & SimpleScreen::_outputs() {
        return m_outputs;
    }

}

