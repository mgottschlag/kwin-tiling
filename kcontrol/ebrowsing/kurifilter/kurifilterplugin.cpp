/* This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kurifilterplugin.h"

KURIFilterPlugin::KURIFilterPlugin(QObject *parent, const char *name, const QString &pname, double pri) : QObject(parent, name) {
    m_strName = pname;
    m_dblPriority = pri;
}

QString KURIFilterPlugin::name() const {
    return m_strName;
}

double KURIFilterPlugin::priority() const {
    return m_dblPriority;
}

bool KURIFilterPlugin::filterURI(QString &uri) const {
    KURL kuri(uri);
    bool filtered = filterURI(kuri);
    //uri = kuri.isMalformed() ? kuri.malformedUrl() : kuri.url();
    return filtered;
}

KCModule *KURIFilterPlugin::configModule(QWidget *, const char *) const {
    return 0;
}

QString KURIFilterPlugin::configName() const {
    return name();
}
