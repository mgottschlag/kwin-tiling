/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#include "launch.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KIcon>
#include <KMenu>

#include <Plasma/DataEngine>
#include <Plasma/Containment>
#include <Plasma/Service>

AppLauncher::AppLauncher(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

void AppLauncher::init(const KConfigGroup &)
{
}

void AppLauncher::contextEvent(QEvent *event)
{
    QPoint screenPos;
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            screenPos = (static_cast<QGraphicsSceneMouseEvent*>(event))->screenPos();
            break;
        case QEvent::GraphicsSceneWheel:
            screenPos = (static_cast<QGraphicsSceneWheelEvent*>(event))->screenPos();
            break;
        default:
            kDebug() << "unexpected event type" << event->type();
            return;
    }


    Plasma::DataEngine *apps = dataEngine("apps");
    if (!apps->isValid()) {
        return;
    }

    KMenu desktopMenu;

    //add the whole kmenu
    Plasma::DataEngine::Data app = dataEngine("apps")->query("/");
    const QStringList sources = app.value("entries").toStringList();
    foreach (const QString &source, sources) {
        addApp(&desktopMenu, source);
    }

    connect(&desktopMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchTo(QAction*)));
    desktopMenu.exec(screenPos);
}

bool AppLauncher::addApp(QMenu *menu, const QString &source)
{
    if (source == "---") {
        menu->addSeparator();
        return false;
    }

    Plasma::DataEngine::Data app = dataEngine("apps")->query(source);

    if (!app.value("display").toBool()) {
        kDebug() << "hidden entry" << source;
        return false;
    }
    QString name = app.value("name").toString();
    if (name.isEmpty()) {
        kDebug() << "failed source" << source;
        return false;
    }

    name.replace("&", "&&"); //escaping
    KIcon icon(app.value("iconName").toString());

    if (app.value("isApp").toBool()) {
        QAction *action = menu->addAction(icon, name);
        action->setData(source);
    } else { //ooh, it's actually a group!
        QMenu *subMenu = menu->addMenu(icon, name);
        bool hasEntries = false;
        foreach (const QString &source, app.value("entries").toStringList()) {
            hasEntries = addApp(subMenu, source) || hasEntries;
        }

        if (!hasEntries) {
            delete subMenu;
            return false;
        }
    }
    return true;
}

void AppLauncher::switchTo(QAction *action)
{
    QString source = action->data().toString();
    kDebug() << source;
    Plasma::Service *service = dataEngine("apps")->serviceForSource(source);
    if (service) {
        service->startOperationCall(service->operationDescription("launch"));
    }
}

#include "launch.moc"
