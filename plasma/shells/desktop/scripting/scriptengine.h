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

#ifndef SCRIPTENGINE
#define SCRIPTENGINE

#include <QScriptEngine>
#include <QScriptValue>

namespace Plasma
{
    class Corona;
} // namespace Plasma

class Containment;

class ScriptEngine : public QScriptEngine
{
    Q_OBJECT

public:
    ScriptEngine(Plasma::Corona *corona, QObject *parent = 0);
    ~ScriptEngine();

    void evaluateScript(const QString &script);

public Q_SLOTS:
    int screenCount() const;
    QRectF screenGeometry(int screen) const;

    QList<int> activityIds() const;
    Containment *activityById(int id) const;
    Containment *activityForScreen(int screen) const;

    QList<int> panelIds() const;
    Containment *panelById(int id) const;

Q_SIGNALS:
    void print(const QString &string);
    void printError(const QString &string);

private:
    void setupEngine();

private Q_SLOTS:
    void exception(const QScriptValue &value);

private:
    Plasma::Corona *m_corona;
    QScriptValue m_scriptSelf;
    Containment *m_dummyContainment;
};

#endif

