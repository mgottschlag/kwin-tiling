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

#ifndef NEWSPAPER_H
#define NEWSPAPER_H

#include <QScriptContext>
#include <QScriptValue>

#include <plasmagenericshell/scripting/containment.h>

namespace WorkspaceScripting
{

class Newspaper : public Containment
{
    Q_OBJECT
    Q_PROPERTY(QStringList configKeys READ configKeys)
    Q_PROPERTY(QStringList configGroups READ configGroups)
    Q_PROPERTY(QStringList currentConfigGroup WRITE setCurrentConfigGroup READ currentConfigGroup)

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString formFactor READ formFactor)
    Q_PROPERTY(QList<int> widgetIds READ widgetIds)
    Q_PROPERTY(int screen READ screen WRITE setScreen)
    Q_PROPERTY(int desktop READ desktop WRITE setDesktop)
    Q_PROPERTY(int id READ id)


public:
    Newspaper(Plasma::Containment *containment, QObject *parent = 0);
    ~Newspaper();

    static QScriptValue addWidgetAt(QScriptContext *context, QScriptEngine *engine);

public Q_SLOTS:
    void remove() { Containment::remove(); }
    void showConfigurationInterface() { Containment::showConfigurationInterface(); }

    // from the applet interface
    QVariant readConfig(const QString &key, const QVariant &def = QString()) const { return Applet::readConfig(key, def); }
    void writeConfig(const QString &key, const QVariant &value) { Applet::writeConfig(key, value); }
    void reloadConfig() { Applet::reloadConfig(); }
};

}

#endif

