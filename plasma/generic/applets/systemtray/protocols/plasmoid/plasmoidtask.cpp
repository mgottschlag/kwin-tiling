/***************************************************************************
 *   plasmoidtask.cpp                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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

#include "plasmoidtask.h"
#include <fixx11h.h>

#include <KIcon>
#include <KIconLoader>

#include <plasma/applet.h>
#include <plasma/popupapplet.h>
#include <plasma/plasma.h>


namespace SystemTray
{

PlasmoidTask::PlasmoidTask(const QString &appletname, int id, QObject *parent, Plasma::Applet *host)
    : Task(parent),
      m_name(appletname),
      m_typeId(appletname),
      m_applet(0),
      m_host(host),
      m_takenByParent(false)
{
    setupApplet(appletname, id);
}


PlasmoidTask::~PlasmoidTask()
{
    emit taskDeleted(m_typeId);
}


bool PlasmoidTask::isEmbeddable() const
{
    return m_applet && !m_takenByParent;
}

bool PlasmoidTask::isValid() const
{
    return !m_name.isEmpty();
}

QString PlasmoidTask::name() const
{
    if (m_applet) {
        return m_applet->name();
    }

    return m_name;
}


QString PlasmoidTask::typeId() const
{
    return m_typeId;
}


QIcon PlasmoidTask::icon() const
{
    return m_icon;
}

Plasma::Applet *PlasmoidTask::host() const
{
    return m_host;
}

QGraphicsWidget* PlasmoidTask::createWidget(Plasma::Applet *host)
{
    if (host != m_host || !m_applet) {
        return 0;
    }

    m_takenByParent = true;
    m_applet->setParent(host);
    m_applet->setParentItem(host);
    m_applet->init();
    m_applet->updateConstraints(Plasma::StartupCompletedConstraint);
    m_applet->flushPendingConstraintsEvents();
    m_applet->updateConstraints(Plasma::AllConstraints);
    m_applet->flushPendingConstraintsEvents();

    // make sure to record it in the configuration so that if we reload from the config,
    // this applet is remembered
    KConfigGroup dummy;
    m_applet->save(dummy);

    connect(m_applet, SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(newAppletStatus(Plasma::ItemStatus)));

    newAppletStatus(m_applet->status());

    connect(m_applet, SIGNAL(configNeedsSaving()), host, SIGNAL(configNeedsSaving()));
    connect(m_applet, SIGNAL(releaseVisualFocus()), host, SIGNAL(releaseVisualFocus()));

    return static_cast<QGraphicsWidget*>(m_applet);
}

void PlasmoidTask::forwardConstraintsEvent(Plasma::Constraints constraints)
{
    if (m_applet) {
        m_applet->updateConstraints(constraints);
        m_applet->flushPendingConstraintsEvents();
    }
}

void PlasmoidTask::setupApplet(const QString &plugin, int id)
{
    m_applet = Plasma::Applet::load(plugin, id);

    if (!m_applet) {
        kDebug() << "Could not load applet" << plugin;
        return;
    }

    m_icon = KIcon(m_applet->icon());

    m_applet->setFlag(QGraphicsItem::ItemIsMovable, false);

    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed(QObject*)));
    m_applet->setBackgroundHints(Plasma::Applet::NoBackground);

    m_applet->setPreferredSize(KIconLoader::SizeSmallMedium+2, KIconLoader::SizeSmallMedium+2);
    kDebug() << m_applet->name() << " Applet loaded";
}

void PlasmoidTask::appletDestroyed(QObject *object)
{
    if (object == m_applet) {
        m_applet = 0;
        deleteLater();
    }
}

void PlasmoidTask::newAppletStatus(Plasma::ItemStatus status)
{
    switch (status) {
    case Plasma::PassiveStatus:
       if (Plasma::PopupApplet *popupApplet = qobject_cast<Plasma::PopupApplet *>(m_applet)) {
           popupApplet->hidePopup();
       }
       setStatus(Task::Passive);
       break;

    case Plasma::ActiveStatus:
       setStatus(Task::Active);
       break;

    case Plasma::NeedsAttentionStatus:
        setStatus(Task::NeedsAttention);
        break;

    default:
    case Plasma::UnknownStatus:
        setStatus(Task::UnknownStatus);
    }

    emit changed(this);
}

}

#include "plasmoidtask.moc"
