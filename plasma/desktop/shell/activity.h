/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QObject>
#include <QList>

class QString;
class QPixmap;
namespace Plasma
{
    class Containment;
} // namespace Plasma

/**
 * This class represents one activity.
 * an activity has an ID and a name, from nepomuk.
 * it also is associated with one or more containments.
 */
class Activity : public QObject
{
    Q_OBJECT
public:
    Activity(const QString &id, QObject *parent = 0);
    ~Activity();
    //FIXME what's the Right Way to set up the initial data?

    QString id();
    QString name();
    QPixmap thumbnail(); //FIXME do we want diff. sizes? updates?

    /**
     * whether this is the currently active activity
     */
    bool isActive();
    /**
     * whether this is one of the activities currently loaded
     */
    bool isRunning();

signals:
    void nameChanged(const QString &name);
//TODO signals for other changes

public slots:
    void setName(const QString &name);
    /**
     * delete the activity forever
     * TODO I wonder what this means for nepomuk...
     */
    void destroy();
    /**
     * make this activity the active one, loading it if necessary
     */
    void activate();

    //TODO stop/start

private:
    QString m_id;
    QString m_name;
    QList<Plasma::Containment*> m_containments;

};

#endif
