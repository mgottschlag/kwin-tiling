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


#include "externalconfiguration.h"



namespace Kephal {

    ExternalConfiguration::ExternalConfiguration(BackendConfigurations * parent)
        : BackendConfiguration(parent)
    {
        m_parent = parent;
    }

    QString ExternalConfiguration::name() const {
        return QString("external");
    }

    bool ExternalConfiguration::isModifiable() const {
        return false;
    }

    bool ExternalConfiguration::isActivated() const {
        return m_parent->activeBackendConfiguration() == this;
    }

    QMap<int, QPoint> ExternalConfiguration::layout() const {
        return QMap<int, QPoint>();
    }

    int ExternalConfiguration::primaryScreen() const {
        return 0;
    }

    void ExternalConfiguration::activate() {
        emit activateExternal();
    }

}

#ifndef NO_KDE
#include "externalconfiguration.moc"
#endif

