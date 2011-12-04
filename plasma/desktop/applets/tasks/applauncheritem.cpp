/***************************************************************************
 *   Copyright (C) 2010 by Anton Kreuzkamp <akreuzkamp@web.de>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "applauncheritem.h"
#include "taskgroupitem.h"

#include <taskmanager/taskactions.h>

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>

// KDE
#include <KAuthorized>
#include <KIconEffect>

#include <Plasma/ToolTipManager>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/PaintUtils>

AppLauncherItem::AppLauncherItem( QGraphicsWidget* parent, Tasks* applet, TaskManager::LauncherItem* launcher)
    : AbstractTaskItem(parent, applet)
{
    m_launcher = launcher;
    setAbstractItem(launcher);
}

void AppLauncherItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && boundingRect().contains(event->pos())) {
        m_launcher->launch();
    }
}

void AppLauncherItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_launcher) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    QList <QAction*> actionList;

    QAction *configAction = m_applet->action("configure");
    if (configAction && configAction->isEnabled()) {
        actionList.append(configAction);
    }

    TaskManager::BasicMenu menu(0, m_launcher, &m_applet->groupManager(), actionList);
    menu.adjustSize();

    if (m_applet->formFactor() != Plasma::Vertical) {
        menu.setMinimumWidth(size().width());
    }

    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    stopWindowHoverEffect();
    menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
}


void AppLauncherItem::updateToolTip()
{
    Plasma::ToolTipContent data(m_launcher->name(),m_launcher->genericName(),m_launcher->icon());
    data.setInstantPopup(true);
    Plasma::ToolTipManager::self()->setContent(this, data);
}

void AppLauncherItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        m_launcher->launch();
    }
    else
    {
        QGraphicsWidget::keyPressEvent(event);
    }
}

void AppLauncherItem::setAdditionalMimeData(QMimeData* mimeData)
{
    if (m_launcher) {
        m_launcher->addMimeData(mimeData);
    }
}

#include "applauncheritem.moc"

