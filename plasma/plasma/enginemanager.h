/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef PLASMA_ENGINE_MANAGER_H
#define PLASMA_ENGINE_MANAGER_H

#include <QHash>

namespace Plasma
{
    class DataEngine;
}

class DataEngineManager
{
    public:
        DataEngineManager();
        ~DataEngineManager();

        Plasma::DataEngine* engine(const QString& name);
        bool loadDataEngine(const QString& name);
        void unloadDataEngine(const QString& name);

    private:
        Plasma::DataEngine::Dict m_engines;
};

#endif // multiple inclusion guard
