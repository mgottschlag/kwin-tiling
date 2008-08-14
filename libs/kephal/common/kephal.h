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


#ifndef KEPHAL_KEPHAL_H
#define KEPHAL_KEPHAL_H

#include <QPoint>

inline uint qHash(const QPoint & key) {
    return ((uint) (key.x() + 32767)) * 65536 + ((uint) (key.y() + 32767));
}


namespace kephal {

    enum Position {
        RightOf,
        LeftOf,
        TopOf,
        BottomOf,
        SameAs
    };
    
}

#define PROPERTY(type, name, setter) \
    private:\
        type m_##name;\
    public:\
        void setter(type name) { m_##name = name; }\
        type name() { return m_##name; }

#endif // KEPHAL_KEPHAL_H

