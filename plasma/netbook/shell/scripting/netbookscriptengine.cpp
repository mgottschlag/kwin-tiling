/*
 *   Copyright 2010 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "netbookscriptengine.h"

#include <plasmagenericshell/scripting/containment.h>
#include <plasmagenericshell/scripting/appinterface.h>

#include "panel.h"

NetbookScriptEngine::NetbookScriptEngine(Plasma::Corona *corona, QObject *parent)
    : ScriptEngine(corona, parent)
{
}

QScriptValue NetbookScriptEngine::wrap(Plasma::Containment *c)
{
    Containment *wrapper = isPanel(c) ? new Panel(c) : new Containment(c);
    return wrap(wrapper);
}

QScriptValue NetbookScriptEngine::wrap(Containment *c)
{
    return ScriptEngine::wrap(c);
}

#include "netbookscriptengine.moc"

