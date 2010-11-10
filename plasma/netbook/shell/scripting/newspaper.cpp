/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "newspaper.h"

#include <KDebug>

#include <Plasma/Corona>
#include <Plasma/Containment>

#include "netview.h"
#include "plasmaapp.h"
#include <plasmagenericshell/scripting/scriptengine.h>
#include <plasmagenericshell/scripting/widget.h>

namespace WorkspaceScripting
{

Newspaper::Newspaper(Plasma::Containment *containment, QObject *parent)
    : Containment(containment, parent)
{
}

Newspaper::~Newspaper()
{
}

QScriptValue Newspaper::addWidgetAt(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 3) {
        return context->throwError(i18n("widgetById requires a name of a widget or a widget object, with the row and column coordinates"));
    }

    Containment *c = qobject_cast<Containment*>(context->thisObject().toQObject());

    if (!c || !c->containment()) {
        return engine->undefinedValue();
    }

    QScriptValue v = context->argument(0);
    int row = context->argument(1).toInt32();
    int column = context->argument(2).toInt32();
    Plasma::Applet *applet = 0;
    if (v.isString()) {
        kWarning()<<QMetaObject::invokeMethod(c->containment(), "addApplet", Qt::DirectConnection,
                           Q_RETURN_ARG(Plasma::Applet *, applet),
                           Q_ARG(QString, v.toString()),
                           Q_ARG(int, row),
                           Q_ARG(int, column));
        if (applet) {
            ScriptEngine *env = ScriptEngine::envFor(engine);
            return env->wrap(applet);
        }
    } else if (Widget *widget = qobject_cast<Widget*>(v.toQObject())) {
        applet = widget->applet();
        QMetaObject::invokeMethod(c->containment(), "addApplet", Qt::DirectConnection,
                           Q_ARG(Plasma::Applet *, applet),
                           Q_ARG(int, row),
                           Q_ARG(int, column));
        c->containment()->addApplet(applet);
        return v;
    }

    return engine->undefinedValue();
}

}

#include "newspaper.moc"


