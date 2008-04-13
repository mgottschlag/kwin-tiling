/*
 *   Copyright 2008 Richard J. Moore <rich@kde.org>
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

#include <QScriptEngine>
#include <QFile>

#include <plasma/abstractrunner.h>
#include <plasma/searchcontext.h>
#include <plasma/searchmatch.h>

#include "runnerscriptqscript.h"


typedef const Plasma::SearchContext* ConstSearchContextStar;
typedef const Plasma::SearchMatch* ConstSearchMatchStar;

Q_DECLARE_METATYPE(Plasma::SearchMatch*)
Q_DECLARE_METATYPE(Plasma::SearchContext*)
Q_DECLARE_METATYPE(ConstSearchContextStar)
Q_DECLARE_METATYPE(ConstSearchMatchStar)

RunnerScriptQScript::RunnerScriptQScript(QObject *parent, const QVariantList &args)
    : RunnerScript(parent)
{
    m_engine = new QScriptEngine( this );
    importExtensions();
}

RunnerScriptQScript::~RunnerScriptQScript()
{
}

Plasma::AbstractRunner* RunnerScriptQScript::runner() const
{
    return RunnerScript::runner();
}

bool RunnerScriptQScript::init()
{
    setupObjects();

    QFile file(mainScript());
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        kWarning() << "Unable to load script file";
        return false;
    }

    QString script = file.readAll();
    kDebug() << "Script says" << script;

    m_engine->evaluate( script );
    if ( m_engine->hasUncaughtException() ) {
        reportError();
        return false;
    }

    return true;
}

void RunnerScriptQScript::match(Plasma::SearchContext *search)
{
    QScriptValue fun = m_self.property( "match" );
    if ( !fun.isFunction() ) {
	kDebug() << "Script: match is not a function, " << fun.toString();
	return;
    }

    QScriptValueList args;
    args << m_engine->toScriptValue(search);

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject( m_self );
    fun.call( m_self, args );
    m_engine->popContext();

    if ( m_engine->hasUncaughtException() ) {
	reportError();
    }
}

void RunnerScriptQScript::exec(const Plasma::SearchContext *search, const Plasma::SearchMatch *action)
{
    QScriptValue fun = m_self.property( "exec" );
    if ( !fun.isFunction() ) {
	kDebug() << "Script: exec is not a function, " << fun.toString();
	return;
    }

    QScriptValueList args;
    args << m_engine->toScriptValue(search);
    args << m_engine->toScriptValue(action);

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject( m_self );
    fun.call( m_self, args );
    m_engine->popContext();

    if ( m_engine->hasUncaughtException() ) {
	reportError();
    }
}

void RunnerScriptQScript::setupObjects()
{
    QScriptValue global = m_engine->globalObject();

    // Expose an applet
    m_self = m_engine->newQObject( this );
    m_self.setScope( global );    

    global.setProperty("runner", m_self);
}

void RunnerScriptQScript::importExtensions()
{
    QStringList extensions;
    extensions << "qt.core" << "qt.gui" << "qt.svg" << "qt.xml" << "org.kde.plasma";
    for (int i = 0; i < extensions.size(); ++i) {
        QString ext = extensions.at(i);
        kDebug() << "importing " << ext << "...";
        QScriptValue ret = m_engine->importExtension(ext);
        if (ret.isError())
            kDebug() << "failed to import extension" << ext << ":" << ret.toString();
    }
    kDebug() << "done importing extensions.";
}

void RunnerScriptQScript::reportError()
{
    kDebug() << "Error: " << m_engine->uncaughtException().toString()
	     << " at line " << m_engine->uncaughtExceptionLineNumber() << endl;
    kDebug() << m_engine->uncaughtExceptionBacktrace();
}

#include "runnerscriptqscript.moc"
