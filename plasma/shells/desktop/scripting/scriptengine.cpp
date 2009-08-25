/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "scriptengine.h"

#include <Plasma/Containment>
#include <Plasma/Corona>

#include "containment.h"

QScriptValue constructQRectFClass(QScriptEngine *engine);

ScriptEngine::ScriptEngine(Plasma::Corona *corona, QObject *parent)
    : QScriptEngine(parent),
      m_corona(corona),
      m_scriptSelf(newQObject(this, QScriptEngine::QtOwnership,
                              QScriptEngine::ExcludeSuperClassProperties |
                              QScriptEngine::ExcludeSuperClassMethods))
{
    Q_ASSERT(m_corona);
    setupEngine();
    connect(this, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exception(QScriptValue)));
}

ScriptEngine::~ScriptEngine()
{
}

int ScriptEngine::screenCount() const
{
    return m_corona->numScreens();
}

QRectF ScriptEngine::screenGeometry(int screen) const
{
    return m_corona->screenGeometry(screen);
}

QList<int> ScriptEngine::activityIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> containments;

    foreach (Plasma::Containment *c, m_corona->containments()) {
        if (!isPanel(c)) {
            containments.append(c->id());
        }
    }

    return containments;
}

QScriptValue ScriptEngine::activityById(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("activityById requires an id"));
    }

    const uint id = context->argument(0).toInt32();
    ScriptEngine *env = envFor(engine);
    foreach (Plasma::Containment *c, env->m_corona->containments()) {
        if (c->id() == id && !isPanel(c)) {
            return wrap(c, engine);
        }
    }

    return wrap(0, engine);
}

QScriptValue ScriptEngine::activityForScreen(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("activityForScreen requires a screen id"));
    }

    const uint screen = context->argument(0).toInt32();
    const uint desktop = context->argumentCount() > 1 ? context->argument(1).toInt32() : -1;
    return wrap(envFor(engine)->m_corona->containmentForScreen(screen, desktop), engine);
}

QScriptValue ScriptEngine::newActivity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("Constructor requires the name of the Activity plugin"));
    }

    return createContainment("desktop", context->argument(0).toString(), context, engine);
}

QScriptValue ScriptEngine::newPanel(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("Constructor requires the name of the Panel plugin"));
    }

    return createContainment("desktop", context->argument(0).toString(), context, engine);
}

QScriptValue ScriptEngine::createContainment(const QString &type, const QString &plugin,
                                             QScriptContext *context, QScriptEngine *engine)
{
    const KPluginInfo::List list = Plasma::Containment::listContainments(type);
    bool exists = false;

    foreach (const KPluginInfo &info, list) {
        if (info.pluginName() == plugin) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        return context->throwError(i18n("Could not find an Activity plugin named %1.", plugin));
    }


    ScriptEngine *env = envFor(engine);
    Plasma::Containment *c = env->m_corona->addContainment(plugin);
    return wrap(c, engine);
}

QScriptValue ScriptEngine::wrap(Plasma::Containment *c, QScriptEngine *engine)
{
    Containment *wrapper = new Containment(c, engine);
    return engine->newQObject(wrapper, QScriptEngine::ScriptOwnership,
                              QScriptEngine::ExcludeSuperClassProperties |
                              QScriptEngine::ExcludeSuperClassMethods);
}

ScriptEngine *ScriptEngine::envFor(QScriptEngine *engine)
{
    QObject *appletObject = engine->globalObject().toQObject();
    Q_ASSERT(appletObject);

    ScriptEngine *env = qobject_cast<ScriptEngine*>(appletObject);
    Q_ASSERT(env);

    return env;
}

QList<int> ScriptEngine::panelIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> panels;

    foreach (Plasma::Containment *c, m_corona->containments()) {
        kDebug() << "checking" << (QObject*)c << isPanel(c);
        if (isPanel(c)) {
            panels.append(c->id());
        }
    }

    return panels;
}

QScriptValue ScriptEngine::panelById(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("activityById requires an id"));
    }

    const uint id = context->argument(0).toInt32();
    ScriptEngine *env = envFor(engine);
    foreach (Plasma::Containment *c, env->m_corona->containments()) {
        if (c->id() == id && isPanel(c)) {
            return wrap(c, engine);
        }
    }

    return wrap(0, engine);
}

void ScriptEngine::setupEngine()
{
    setGlobalObject(m_scriptSelf);
    m_scriptSelf.setProperty("QRectF", constructQRectFClass(this));
    m_scriptSelf.setProperty("Activity", newFunction(ScriptEngine::newActivity));
    m_scriptSelf.setProperty("Panel", newFunction(ScriptEngine::newPanel));
    m_scriptSelf.setProperty("activityById", newFunction(ScriptEngine::activityById));
    m_scriptSelf.setProperty("activityForScreen", newFunction(ScriptEngine::activityForScreen));
    m_scriptSelf.setProperty("panelById", newFunction(ScriptEngine::panelById));
}

bool ScriptEngine::isPanel(const Plasma::Containment *c)
{
    return c->containmentType() == Plasma::Containment::PanelContainment ||
           c->containmentType() == Plasma::Containment::CustomPanelContainment;
}

void ScriptEngine::evaluateScript(const QString &script)
{
    //kDebug() << "evaluating" << m_editor->toPlainText();
    evaluate(script);
    if (hasUncaughtException()) {
        kDebug() << "catch the exception!";
        QString error = i18n("Error: %1 at line %2\n\nBacktrace:\n%3",
                             uncaughtException().toString(),
                             QString::number(uncaughtExceptionLineNumber()),
                             uncaughtExceptionBacktrace().join("\n  "));
        emit printError(error);
    }
}

void ScriptEngine::exception(const QScriptValue &value)
{
    //kDebug() << "exception caught!" << value.toVariant();
    emit printError(value.toVariant().toString());
}

#include "scriptengine.moc"

