/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "greeterplugin.h"
#include "greeter.h"
#include "sessions.h"

// KDE
#include <KDE/KUser>
// Qt
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>

void GreeterPlugin::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    QDeclarativeExtensionPlugin::initializeEngine(engine, uri);
    QDeclarativeContext *context = engine->rootContext();
    KUser user;
    context->setContextProperty("kscreenlocker_userName", user.property(KUser::FullName).toString());
}

void GreeterPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.kscreenlocker"));

    qmlRegisterType<ScreenLocker::GreeterItem>(uri, 1, 0, "GreeterItem");
    qmlRegisterType<ScreenLocker::KeyboardItem>(uri, 1, 0, "KeyboardItem");
    qmlRegisterType<ScreenLocker::SessionSwitching>(uri, 1, 0, "Sessions");
    qmlRegisterType<QAbstractItemModel>();
}

#include "greeterplugin.moc"
