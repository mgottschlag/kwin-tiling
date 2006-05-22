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

#include "enginemanager.h"

DataEngineManager::DataEngineManager()
{
}

~DataEngineManager::DataEngineManager()
{
    foreach (Plasma::DataEngine* engine, engines)
    {
        delete engine;
    }
    engines.clear();
}

Plasma::DataEngine* DataEngineManager::engine(const QString& name)
{
    Plasma::DataEngine::Dict::iterator it = engines.find(name);
    if (it != engines.end())
    {
        // ref and return the engine
        //Plasma::DataEngine *engine = *it;
        return *it;
    }

    // load the engine, add it to the engines
    KService::List offers = KTrader::self()->query("Plasma/Engine");

    if (offers.isEmpty())
    {
        return 0;
    }

    int errorCode = 0;
    Plasma::DataEngine *engine =
            KParts::ComponentFactory::createInstanceFromService<Plasma::DataEngine>
            (offers.first(), 0, 0, QStringList(), &errorCode);

    if (!engine)
    {
        return 0;
    }

    engine->init();
    engines[name] = engine;
    return engine;
}
