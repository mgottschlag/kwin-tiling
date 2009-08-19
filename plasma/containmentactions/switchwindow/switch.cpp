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

#include "switch.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KMenu>
#include <KWindowSystem>

#include <Plasma/DataEngine>
#include <Plasma/Service>

SwitchWindow::SwitchWindow(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
    setHasConfigurationInterface(true);
}

void SwitchWindow::init(const KConfigGroup &config)
{
    m_mode = (MenuMode)config.readEntry("mode", (int)AllFlat);
}

QWidget* SwitchWindow::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);
    widget->setWindowTitle(i18n("Configure Switch Window Plugin"));
    switch (m_mode) {
        case AllFlat:
            m_ui.flatButton->setChecked(true);
            break;
        case DesktopSubmenus:
            m_ui.subButton->setChecked(true);
            break;
        case CurrentDesktop:
            m_ui.curButton->setChecked(true);
            break;
    }
    return widget;
}

void SwitchWindow::configurationAccepted()
{
    if (m_ui.flatButton->isChecked()) {
        m_mode = AllFlat;
    } else if (m_ui.subButton->isChecked()) {
        m_mode = DesktopSubmenus;
    } else {
        m_mode = CurrentDesktop;
    }
}

void SwitchWindow::save(KConfigGroup &config)
{
    config.writeEntry("mode", (int)m_mode);
}

void SwitchWindow::contextEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMouseRelease:
            contextEvent(dynamic_cast<QGraphicsSceneMouseEvent*>(event));
            break;
        case QEvent::GraphicsSceneWheel:
            wheelEvent(dynamic_cast<QGraphicsSceneWheelEvent*>(event));
            break;
        default:
            break;
    }
}

QMenu *SwitchWindow::makeMenu()
{
    Plasma::DataEngine *tasks = dataEngine("tasks");
    if (! tasks->isValid()) {
        return 0;
    }

    QMultiHash<int, QAction*> desktops;
    KMenu *desktopMenu = new KMenu();

    //make all the window actions
    foreach (const QString &source, tasks->sources()) {
        Plasma::DataEngine::Data window = tasks->query(source);
        if (window.value("startup").toBool()) {
            kDebug() << "skipped fake task" << source;
            continue;
        }

        QString name = window.value("visibleNameWithState").toString();
        if (name.isEmpty()) {
            kDebug() << "failed source" << source;
            continue;
        }

        QAction *action = new QAction(name, desktopMenu);
        action->setIcon(window.value("icon").value<QIcon>());
        action->setData(source);
        desktops.insert(window.value("desktop").toInt(), action);
    }

    //sort into menu
    if (m_mode == CurrentDesktop) {
        int currentDesktop = KWindowSystem::currentDesktop();
        desktopMenu->addTitle(i18n("Windows"));
        desktopMenu->addActions(desktops.values(currentDesktop));
        desktopMenu->addActions(desktops.values(-1));
    } else {
        int numDesktops = KWindowSystem::numberOfDesktops();
        if (m_mode == AllFlat) {
            for (int i = 1; i <= numDesktops; ++i) {
                QString name = KWindowSystem::desktopName(i);
                name = QString("%1: %2").arg(i).arg(name);
                desktopMenu->addTitle(name);
                desktopMenu->addActions(desktops.values(i));
            }
            desktopMenu->addTitle(i18n("All Desktops"));
            desktopMenu->addActions(desktops.values(-1));
        } else { //submenus
            for (int i = 1; i <= numDesktops; ++i) {
                QString name = KWindowSystem::desktopName(i);
                name = QString("%1: %2").arg(i).arg(name);
                KMenu *subMenu = new KMenu(name, desktopMenu);
                subMenu->addActions(desktops.values(i));
                desktopMenu->addMenu(subMenu);
            }
            KMenu *subMenu = new KMenu(i18n("All Desktops"), desktopMenu);
            subMenu->addActions(desktops.values(-1));
            desktopMenu->addMenu(subMenu);
        }
    }

    connect(desktopMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchTo(QAction*)));
    return desktopMenu;
}

void SwitchWindow::contextEvent(QGraphicsSceneMouseEvent *event)
{
    QMenu *desktopMenu = makeMenu();
    if (desktopMenu) {
        desktopMenu->exec(event->screenPos());
    }
}

QList<QAction*> SwitchWindow::contextualActions()
{
    QList<QAction*> list;
    QMenu *menu = makeMenu();
    if (menu) {
        QAction *action = new QAction(this); //FIXME I hope this doesn't leak
        action->setMenu(menu);
        menu->setTitle(i18n("Windows"));
        list << action;
    }
    return list;
}

void SwitchWindow::switchTo(QAction *action)
{
    QString source = action->data().toString();
    kDebug() << source;
    Plasma::Service *service = dataEngine("tasks")->serviceForSource(source);
    if (service) {
        service->startOperationCall(service->operationDescription("activateRaiseOrIconify"));
    }
}

void SwitchWindow::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    //TODO somehow find the "next" or "previous" window
    //without changing hte window order (don't want to always go between two windows)
}


#include "switch.moc"
