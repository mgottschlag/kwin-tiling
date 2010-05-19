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

#ifndef NETBOOKSCRIPTENGINE_H
#define NETBOOKSCRIPTENGINE_H

#include <plasmagenericshell/scripting/scriptengine.h>

namespace WorkspaceScripting
{

class NetbookScriptEngine : public ScriptEngine
{
    Q_OBJECT

public:
    NetbookScriptEngine(Plasma::Corona *corona, QObject *parent = 0);
    QScriptValue wrap(Plasma::Containment *c);
    QScriptValue wrap(Containment *c);
};

}

#endif

