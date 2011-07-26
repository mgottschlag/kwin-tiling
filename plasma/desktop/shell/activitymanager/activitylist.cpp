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
#include "kworkspace/kactivitycontroller.h"

#include <QHash>

#include <Plasma/Containment>
#include <Plasma/Corona>

#include <KService>
#include <KServiceTypeTrader>

ActivityList::ActivityList(Plasma::Location location, QGraphicsItem *parent)
    : AbstractIconList(location, parent),
      m_activityController(new KActivityController(this)),
      m_scheduleHideOnAdd(0)
{
    QStringList activities = m_activityController->listActivities();
    foreach (const QString &activity, activities) {
        createActivityIcon(activity);
    }

    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate");
    foreach (const KService::Ptr &service, templates) {
        if (!service->property("X-Plasma-ContainmentLayout-ShowAsExisting", QVariant::Bool).toBool()) continue;

        KConfig config("plasma-desktoprc");
        KConfigGroup group(&config, "ActivityManager HiddenTemplates");

        if (group.readEntry(service->storageId(), false)) continue;

        createActivityIcon(service->name(), service->icon(), service->storageId());
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

void ActivityList::createActivityIcon(const QString &name, const QString &iconName, const QString &plugin)
{
    ActivityIcon *icon = new ActivityIcon(name, iconName, plugin);

    connect(icon, SIGNAL(requestsRemoval(bool)),
            this, SLOT(templateHidden(bool)));

    addIcon(icon);
    m_allAppletsHash.insert("null:" + name, icon);
    // m_allAppletsHash.insert(id, icon);
    // connect(icon->activity(), SIGNAL(stateChanged()), this, SLOT(updateClosable()));
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

    // Syncing removal of template activity andd addition of a
    // new activity based on it
    if (m_scheduleHideOnAdd) {
        hideIcon(m_scheduleHideOnAdd);
        m_allAppletsHash.remove(m_allAppletsHash.key(m_scheduleHideOnAdd));
        m_scheduleHideOnAdd = 0;
    }

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

        if (icon && icon->activity() && icon->activity()->state() == KActivityInfo::Running) {
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

void ActivityList::templateHidden(bool immediate)
{
    ActivityIcon * icon = qobject_cast < ActivityIcon * > (sender());

    if (!icon) return;

    if (immediate) {
        hideIcon(icon);
        m_allAppletsHash.remove(m_allAppletsHash.key(icon));

    } else {
        m_scheduleHideOnAdd = icon;
    }
}

