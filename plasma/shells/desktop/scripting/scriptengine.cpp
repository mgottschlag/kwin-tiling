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

#include <Plasma/Corona>

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

void ScriptEngine::setupEngine()
{
    setGlobalObject(m_scriptSelf);
    m_scriptSelf.setProperty("QRectF", constructQRectFClass(this));
}

void ScriptEngine::evaluateScript(const QString &script)
{
    //kDebug() << "evaluating" << m_editor->toPlainText();
    evaluate(script);
    if (hasUncaughtException()) {
        kDebug() << "catch the exception!";
        QString error = i18n("Error: %1 at line %2<p>Backtrace:<br>%3",
                             uncaughtException().toString(),
                             QString::number(uncaughtExceptionLineNumber()),
                             uncaughtExceptionBacktrace().join("<br>"));
        emit printError(error);
    }
}

void ScriptEngine::exception(const QScriptValue &value)
{
    //kDebug() << "exception caught!" << value.toVariant();
    emit printError(value.toVariant().toString());
}

#include "scriptengine.moc"

