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
#include "kactivitymanager_p.h"

#ifdef HAVE_SOPRANO_PLUGIN_RAPTORPARSER
#include "nfo.h"
#include "nco.h"
#include "pimo.h"
#endif

// Private

KActivityInfo::Private::Private(KActivityInfo *info, const QString &activityId)
    : q(info),
      state(KActivityInfo::Invalid),
      id(activityId)
{
    KActivityManager::self();
}

KUrl KActivityInfo::Private::urlForType(KActivityInfo::ResourceType resourceType) const
{
    switch (resourceType) {
#ifdef HAVE_SOPRANO_PLUGIN_RAPTORPARSER
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
#endif
        default:
            return KUrl(QString());
    }
}

#define IMPLEMENT_SIGNAL_HANDLER(ORIGINAL,INTERNAL) \
    void KActivityInfo::Private::INTERNAL(const QString & _id) const  \
    {                                                                 \
        if (id == _id) emit q->INTERNAL();                            \
    }

IMPLEMENT_SIGNAL_HANDLER(ActivityAdded,added)
IMPLEMENT_SIGNAL_HANDLER(ActivityRemoved,removed)
IMPLEMENT_SIGNAL_HANDLER(ActivityStarted,started)
IMPLEMENT_SIGNAL_HANDLER(ActivityStopped,stopped)
IMPLEMENT_SIGNAL_HANDLER(ActivityChanged,infoChanged)

#undef IMPLEMENT_SIGNAL_HANDLER

void KActivityInfo::Private::activityStateChanged(const QString & idChanged, int newState)
{
    if (idChanged == id) {
        state = static_cast<KActivityInfo::State>(newState);
        emit q->stateChanged(state);
    }
}

// KActivityInfo
KActivityInfo::KActivityInfo(const QString &activityId, QObject *parent)
    : QObject(parent),
      d(new Private(this, activityId))
{
    d->id = activityId;
    connect(KActivityManager::self(), SIGNAL(ActivityStateChanged(QString,int)),
            this, SLOT(activityStateChanged(QString,int)));

    connect(KActivityManager::self(), SIGNAL(ActivityChanged(QString)),
            this, SLOT(infoChanged(QString)));

    connect(KActivityManager::self(), SIGNAL(ActivityAdded(QString)),
            this, SLOT(added(QString)));

    connect(KActivityManager::self(), SIGNAL(ActivityRemoved(QString)),
            this, SLOT(removed(QString)));

    connect(KActivityManager::self(), SIGNAL(ActivityStarted(QString)),
            this, SLOT(started(QString)));

    connect(KActivityManager::self(), SIGNAL(ActivityStopped(QString)),
            this, SLOT(stopped(QString)));
}

KActivityInfo::~KActivityInfo()
{
    delete d;
}

bool KActivityInfo::isValid() const
{
    return (state() != Invalid);
}

void KActivityInfo::associateResource(const KUrl & resource, ResourceType resourceType)
{
    associateResource(resource, d->urlForType(resourceType));
}

void KActivityInfo::associateResource(const KUrl & resource, const KUrl & resourceType)
{
    // TODO:
    // Private::s_store
    //     ->associateResource(d->id, resource.url(), resourceType.url());
}

void KActivityInfo::disassociateResource(const KUrl & resource)
{
    // TODO:
    // Private::s_store
    //     ->disassociateResource(d->id, resource.url());
}

QList < KUrl > KActivityInfo::associatedResources(ResourceType resourceType) const
{
    return associatedResources(d->urlForType(resourceType));
}

QList < KUrl > KActivityInfo::associatedResources(const KUrl & resourceType) const
{
    QList < KUrl > result;

    // TODO:
    // QDBusReply < QStringList > reply = Private::s_store
    //     ->associatedResources(d->id, resourceType.url());

    // if (reply.isValid()) {
    //     QStringList associatedResources = reply.value();

    //     foreach (const QString & uri, associatedResources) {
    //         result << KUrl(uri);
    //     }
    // }

    return result;
}

// macro defines a shorthand for validating and returning a d-bus result
// @param REPLY_TYPE type of the d-bus result
// @param CAST_TYPE type to which to cast the result
// @param METHOD invocation of the d-bus method
#define KACTIVITYINFO_DBUS_CAST_RETURN(REPLY_TYPE, CAST_TYPE, METHOD)  \
    QDBusReply < REPLY_TYPE > dbusReply = METHOD;                      \
    if (dbusReply.isValid()) {                                         \
        return (CAST_TYPE)(dbusReply.value());                         \
    } else {                                                           \
        return CAST_TYPE();                                            \
    }


KUrl KActivityInfo::uri() const
{
    // TODO:
    return KUrl();
    // KACTIVITYINFO_DBUS_CAST_RETURN(
    //     QString, KUrl, Private::s_store->uri(d->id));
}

KUrl KActivityInfo::resourceUri() const
{
    // TODO:
    return KUrl();
    // KACTIVITYINFO_DBUS_CAST_RETURN(
    //     QString, KUrl, Private::s_store->resourceUri(d->id));
}

QString KActivityInfo::id() const
{
    return d->id;
}

QString KActivityInfo::name() const
{
    KACTIVITYINFO_DBUS_CAST_RETURN(
        QString, QString, KActivityManager::self()->ActivityName(d->id));
}

QString KActivityInfo::icon() const
{
    KACTIVITYINFO_DBUS_CAST_RETURN(
        QString, QString, KActivityManager::self()->ActivityIcon(d->id));
}

KActivityInfo::State KActivityInfo::state() const
{
    if (d->state == Invalid) {
        QDBusReply < int > dbusReply = KActivityManager::self()->ActivityState(d->id);

        if (dbusReply.isValid()) {
            d->state = (State)(dbusReply.value());
        }
    }

    return d->state;
}

QString KActivityInfo::name(const QString & id)
{
    KACTIVITYINFO_DBUS_CAST_RETURN(
            QString, QString, KActivityManager::self()->ActivityName(id));
}

#undef KACTIVITYINFO_DBUS_CAST_RETURN

KActivityInfo::Availability KActivityInfo::availability() const
{
    Availability result = Nothing;

    if (!KActivityManager::isActivityServiceRunning()) {
        return result;
    }

    if (KActivityManager::self()->ListActivities().value().contains(d->id)) {
        result = BasicInfo;

        if (KActivityManager::self()->IsBackstoreAvailable()) {
            result = Everything;
        }
    }

    return result;
}

#include "kactivityinfo.moc"

