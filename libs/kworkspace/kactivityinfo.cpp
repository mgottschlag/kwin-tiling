/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KDebug>

#include "kactivityinfo.h"
#include "kactivityinfo_p.h"

#include "nfo.h"
#include "nco.h"
#include "pimo.h"

// Private

KActivityInfoStaticPrivate * KActivityInfoStaticPrivate::m_instance = NULL;

KActivityInfoStaticPrivate::KActivityInfoStaticPrivate()
    : QObject(), m_store(NULL), m_manager(NULL)
{
    connect(manager(), SIGNAL(ActivityNameChanged(const QString &, const QString &)),
        this, SLOT(activityNameChanged(const QString &, const QString &)));
}

KActivityInfoStaticPrivate * KActivityInfoStaticPrivate::self()
{
    if (!m_instance) {
        m_instance = new KActivityInfoStaticPrivate();
    }

    return m_instance;
}

org::kde::ActivityManager * KActivityInfoStaticPrivate::manager()
{
    if (!m_manager) {
        m_manager = new org::kde::ActivityManager(
                "org.kde.ActivityManager",
                "/ActivityManager",
                QDBusConnection::sessionBus()
                );
    }

    return m_manager;
}

org::kde::nepomuk::services::NepomukActivitiesService * KActivityInfoStaticPrivate::store()
{
    if (!m_store) {
        m_store = new org::kde::nepomuk::services::NepomukActivitiesService(
                "org.kde.nepomuk.services.nepomukactivitiesservice",
                "/nepomukactivitiesservice",
                QDBusConnection::sessionBus()
                );
    }

    return m_store;
}

KUrl KActivityInfoStaticPrivate::urlForType(KActivityInfo::ResourceType resourceType)
{
    switch (resourceType) {
        case KActivityInfo::DocumentResource:
            return Ontologies::nfo::Document();

        case KActivityInfo::FolderResource:
            return Ontologies::nfo::Folder();

        case KActivityInfo::ApplicationResource:
            return Ontologies::nfo::Application();

        case KActivityInfo::ContactResource:
            return Ontologies::nco::Contact();

        case KActivityInfo::LocationResource:
            return Ontologies::pimo::Location();

        default:
            return KUrl(QString());
    }
}

void KActivityInfoStaticPrivate::activityNameChanged(const QString & id, const QString & name)
{
    if (!infoObjects.contains(id)) {
        return;
    }

    kDebug() <<
    KActivityInfo::staticMetaObject.invokeMethod(
        infoObjects[id],
        "nameChanged",
        Qt::QueuedConnection,
        Q_ARG(QString, name)
    );

}

// KActivityInfo

KActivityInfo::KActivityInfo(const QString & activityId)
    : d(new Private())
{
    d->id = activityId;
}

KActivityInfo::~KActivityInfo()
{
    delete d;
}

KActivityInfo * KActivityInfo::forActivity(const QString & id)
{
    if (!KActivityInfoStaticPrivate::self()->store() ||
            !KActivityInfoStaticPrivate::self()->store()->listAvailable().value().contains(id)) {
        kDebug() << (void *)KActivityInfoStaticPrivate::self()->store() << id << "not found in"
            << KActivityInfoStaticPrivate::self()->store()->listAvailable().value();
        return 0;
    }

    if (!KActivityInfoStaticPrivate::self()->infoObjects[id]) {
        KActivityInfoStaticPrivate::self()->infoObjects[id] = new KActivityInfo(id);
    }

    return KActivityInfoStaticPrivate::self()->infoObjects[id];
}

void KActivityInfo::associateResource(const KUrl & resource, ResourceType resourceType)
{
    associateResource(resource, KActivityInfoStaticPrivate::self()->urlForType(resourceType));
}

void KActivityInfo::associateResource(const KUrl & resource, const KUrl & resourceType)
{
    KActivityInfoStaticPrivate::self()->store()
        ->associateResource(d->id, resource.url(), resourceType.url());
}

void KActivityInfo::disassociateResource(const KUrl & resource)
{
    KActivityInfoStaticPrivate::self()->store()
        ->disassociateResource(d->id, resource.url());
}

QList < KUrl > KActivityInfo::associatedResources(ResourceType resourceType) const
{
    return associatedResources(KActivityInfoStaticPrivate::self()->urlForType(resourceType));
}

QList < KUrl > KActivityInfo::associatedResources(const KUrl & resourceType) const
{
    QList < KUrl > result;
    QStringList associatedResources = KActivityInfoStaticPrivate::self()->store()
        ->associatedResources(d->id, resourceType.url());

    foreach (const QString & uri, associatedResources) {
        result << KUrl(uri);
    }

    return result;
}

KUrl KActivityInfo::uri() const
{
    return KUrl(KActivityInfoStaticPrivate::self()->store()->uri(d->id));
}

KUrl KActivityInfo::resourceUri() const
{
    return KUrl(KActivityInfoStaticPrivate::self()->store()->resourceUri(d->id));
}

QString KActivityInfo::id() const
{
    return d->id;
}

QString KActivityInfo::name() const
{
    return KActivityInfoStaticPrivate::self()->manager()->ActivityName(d->id);
}

QString KActivityInfo::icon() const
{
    return KActivityInfoStaticPrivate::self()->manager()->ActivityIcon(d->id);
}

QString KActivityInfo::name(const QString & id)
{
    return KActivityInfoStaticPrivate::self()->manager()->ActivityName(id);
}

