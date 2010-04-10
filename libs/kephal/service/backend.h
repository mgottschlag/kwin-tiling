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


#ifndef KEPHAL_BACKEND_H
#define KEPHAL_BACKEND_H

#define PROPERTY(type, name, setter) \
    private:\
        type m_##name;\
    public:\
        void setter(type name) { m_##name = name; }\
        type name() { return m_##name; }


#define INVALID_CONFIGURATION(desc) kDebug() << "INVALID CONFIGURATION:" << desc;

#define CONFIGURATION_NOT_FOUND(name) kDebug() << "CONFIGURATION NOT FOUND:" << name;

#define FIX_ME(desc) kDebug() << "FIXME:" << desc;

#define OPERATION_FAILED(desc) kDebug() << "OPERATION FAILED:" << desc;

#endif // KEPHAL_BACKEND_H
