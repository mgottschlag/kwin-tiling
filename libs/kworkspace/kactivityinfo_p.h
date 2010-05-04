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

#ifndef ACTIVITY_INFO_PH
#define ACTIVITY_INFO_PH

#include "activitymanager_interface.h"
#include "nepomukactivitiesservice_interface.h"
#include "kactivityinfo.h"

class KActivityInfoStaticPrivate: public QObject {
    Q_OBJECT

public:
    static KActivityInfoStaticPrivate * self();

    QHash < QString , KActivityInfo * > infoObjects;

    org::kde::ActivityManager * manager();
    org::kde::nepomuk::services::NepomukActivitiesService * store();

    KUrl urlForType(KActivityInfo::ResourceType resourceType);

private Q_SLOTS:
    void activityNameChanged(const QString & id, const QString & name);

private:
    KActivityInfoStaticPrivate();

    static KActivityInfoStaticPrivate * m_instance;

    org::kde::nepomuk::services::NepomukActivitiesService * m_store;
    org::kde::ActivityManager * m_manager;

};

class KActivityInfo::Private {
public:
    QString id;

    void emitActivityNameChanged();

};

#endif // ACTIVITY_INFO_PH
