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

#include <QScriptValueIterator>
#include <QEventLoop>
#include <QTimer>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

#include "containment.h"
#include "widget.h"

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

    return engine->undefinedValue();
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
    return createContainment("desktop", "desktop", context, engine);
}

QScriptValue ScriptEngine::newPanel(QScriptContext *context, QScriptEngine *engine)
{
    return createContainment("panel", "panel", context, engine);
}

QScriptValue ScriptEngine::createContainment(const QString &type, const QString &defaultPlugin,
                                             QScriptContext *context, QScriptEngine *engine)
{
    QString plugin = context->argumentCount() > 0 ? context->argument(0).toString() :
                                                    defaultPlugin;

    bool exists = false;
    const KPluginInfo::List list = Plasma::Containment::listContainmentsOfType(type);
    foreach (const KPluginInfo &info, list) {
        if (info.pluginName() == plugin) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        return context->throwError(i18n("Could not find an %1 plugin named %2.", type, plugin));
    }


    ScriptEngine *env = envFor(engine);
    Plasma::Containment *c = env->m_corona->addContainment(plugin);
    if (c) {
        c->updateConstraints(Plasma::StartupCompletedConstraint);
        c->flushPendingConstraintsEvents();
    }

    return wrap(c, engine);
}

QScriptValue ScriptEngine::wrap(Plasma::Applet *w, QScriptEngine *engine)
{
    Widget *wrapper = new Widget(w);
    QScriptValue v = engine->newQObject(wrapper, QScriptEngine::ScriptOwnership,
                                        QScriptEngine::ExcludeSuperClassProperties |
                                        QScriptEngine::ExcludeSuperClassMethods);
    return v;
}

QScriptValue ScriptEngine::wrap(Plasma::Containment *c, QScriptEngine *engine)
{
    Containment *wrapper = new Containment(c);
    QScriptValue v = engine->newQObject(wrapper, QScriptEngine::ScriptOwnership,
                                        QScriptEngine::ExcludeSuperClassProperties |
                                        QScriptEngine::ExcludeSuperClassMethods);
    v.setProperty("widgetById", engine->newFunction(Containment::widgetById));
    v.setProperty("addWidget", engine->newFunction(Containment::addWidget));
    /*
    TODO: this does not actually work, look into why
    if (!isPanel(c)) {
        // remove all items we don't want showing to non-panel containments
        QScriptValueIterator it(v);
        QSet<QString> blacklist;
        blacklist << "alignment";
        while (it.hasNext()) {
            it.next();
            kDebug() << it.name();
            if (blacklist.contains(it.name())) {
                kDebug() << "removing" << it.name();
                it.remove();
                v.setProperty(it.name(), QScriptValue());
            }
        }
    }
    */

    return v;
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
        //kDebug() << "checking" << (QObject*)c << isPanel(c);
        if (isPanel(c)) {
            panels.append(c->id());
        }
    }

    return panels;
}

void ScriptEngine::lockCorona(bool locked)
{
    m_corona->setImmutability(locked ? Plasma::UserImmutable : Plasma::Mutable);
}

bool ScriptEngine::coronaLocked() const
{
    return m_corona->immutability() != Plasma::Mutable;
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

    return engine->undefinedValue();
}

void ScriptEngine::setupEngine()
{
    QScriptValue v = globalObject();
    QScriptValueIterator it(v);
    while (it.hasNext()) {
        it.next();
        // we provide our own print implementation, but we want the rest
        if (it.name() != "print") {
            m_scriptSelf.setProperty(it.name(), it.value());
        }
    }

    m_scriptSelf.setProperty("QRectF", constructQRectFClass(this));
    m_scriptSelf.setProperty("Activity", newFunction(ScriptEngine::newActivity));
    m_scriptSelf.setProperty("Panel", newFunction(ScriptEngine::newPanel));
    m_scriptSelf.setProperty("activityById", newFunction(ScriptEngine::activityById));
    m_scriptSelf.setProperty("activityForScreen", newFunction(ScriptEngine::activityForScreen));
    m_scriptSelf.setProperty("panelById", newFunction(ScriptEngine::panelById));

    setGlobalObject(m_scriptSelf);
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

void ScriptEngine::sleep(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}

#include "scriptengine.moc"

