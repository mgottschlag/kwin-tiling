/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

#ifndef ACTIVITY_ENGINE_H
#define ACTIVITY_ENGINE_H

#include <Plasma/Service>
#include <Plasma/DataEngine>

class ActivityService;
class KActivityController;
class KActivityInfo;

class ActivityEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    ActivityEngine(QObject* parent, const QVariantList& args);
    Plasma::Service *serviceForSource(const QString &source);
    void init();

public slots:
    void activityAdded(const QString &id);
    void activityRemoved(const QString &id);
    void currentActivityChanged(const QString &id);

    void activityNameChanged(const QString &newName);

private:
    void insertActivity(const QString &id);

    KActivityController *m_activityController;
    QString m_currentActivity;

    friend class ActivityService;
};

#endif // SEARCHLAUNCH_ENGINE_H
