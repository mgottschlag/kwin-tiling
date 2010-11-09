/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activitylist.h"

#include "activity.h"
#include "plasmaapp.h"
#include "kactivitycontroller.h"

#include <QHash>

#include <Plasma/Containment>
#include <Plasma/Corona>

ActivityList::ActivityList(Plasma::Location location, QGraphicsItem *parent)
    : AbstractIconList(location, parent),
      m_activityController(new KActivityController(this))
{
    QStringList activities = m_activityController->listActivities();
    foreach (const QString &activity, activities) {
        createActivityIcon(activity);
    }

    updateClosable();

    /*
    if (m_allAppletsHash.count() == 1) {
        ActivityIcon *icon = qobject_cast<ActivityIcon*>(m_allAppletsHash.values().first());
        if (icon) {
            icon->setClosable(false);
        }
    }*/
    //TODO:
    //-do something about sorting and filtering (most recent first?)

    connect(m_activityController, SIGNAL(activityAdded(const QString &)), this, SLOT(activityAdded(const QString &)));
    connect(m_activityController, SIGNAL(activityRemoved(const QString &)), this, SLOT(activityRemoved(const QString &)));

    updateList();
}

ActivityList::~ActivityList()
{
}

void ActivityList::createActivityIcon(const QString &id)
{
    ActivityIcon *icon = new ActivityIcon(id);
    addIcon(icon);
    m_allAppletsHash.insert(id, icon);
    connect(icon->activity(), SIGNAL(stateChanged()), this, SLOT(updateClosable()));
}
/*
void AppletsListWidget::appletIconDoubleClicked(AbstractIcon *icon)
{
    emit(appletDoubleClicked(static_cast<AppletIconWidget*>(icon)->appletItem()));
}
*/

void ActivityList::updateVisibleIcons()
{
}

void ActivityList::setSearch(const QString &searchString)
{
    foreach (Plasma::AbstractIcon *icon, m_allAppletsHash) {
        icon->setVisible(icon->name().contains(searchString, Qt::CaseInsensitive));
    }
}

void ActivityList::activityAdded(const QString &id)
{
    //kDebug() << id;
    /*
    if (m_allAppletsHash.count() == 1) {
        ActivityIcon *icon = qobject_cast<ActivityIcon*>(m_allAppletsHash.values().first());
        if (icon) {
            icon->setClosable(true);
        }
    }
    */
    createActivityIcon(id);
    updateList();
}

void ActivityList::activityRemoved(const QString &id)
{
    ActivityIcon *icon = qobject_cast<ActivityIcon *>(m_allAppletsHash.take(id));

    if (icon) {
        icon->activityRemoved();
    }
/*
    if (m_allAppletsHash.count() == 1) {
        ActivityIcon *icon = qobject_cast<ActivityIcon*>(m_allAppletsHash.values().first());
        if (icon) {
            icon->setClosable(false);
        }
    }
*/
    updateList();
}

void ActivityList::updateClosable()
{
    ActivityIcon * running = 0;
    bool twoRunning = false;

    foreach (Plasma::AbstractIcon *i, m_allAppletsHash) {
        ActivityIcon *icon = qobject_cast<ActivityIcon*>(i);

        if (icon && icon->activity()->state() == KActivityInfo::Running) {
            if (running) {
                //found two, no worries
                twoRunning = true;
                break;
            } else {
                running = icon;
            }
        }
    }

    if (twoRunning) {
        foreach (Plasma::AbstractIcon *i, m_allAppletsHash) {
            qobject_cast < ActivityIcon * > (i)->setClosable(true);
        }

    } else if (running) {
        running->setClosable(false);
    }
}
