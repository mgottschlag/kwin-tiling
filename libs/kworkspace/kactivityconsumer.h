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

#ifndef ACTIVITY_CONSUMER_H
#define ACTIVITY_CONSUMER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <KUrl>
#include <QStringList>

#include <kdemacros.h>

class KActivityConsumerPrivate;
/**
 * Contextual information can be, from the user's point of view, divided
 * into three aspects - "who am I?", "where am I?" (what are my surroundings?)
 * and "what am I doing?".
 *
 * Activities deal with the last one - "what am I doing?". The current activity
 * refers to what the user is doing at the moment, while the other activities represent
 * things that he/she was doing before, and probably will be doing again.
 *
 * Activity is an abstract concept whose meaning can differ from one user to another.
 * Typical examples of activities are "developing a KDE project", "studying the
 * 19th century art", "composing music", "lazing on a Sunday afternoon" etc.
 *
 * Every activity can have applications, documents, or other types of resources
 * assigned to it.
 *
 * KActivityConsumer provides an entry-level API for supporting activities in an
 * application - to react to the changes to the current activity as well as
 * registering the resources with its windows.
 *
 * Resource can be anything that is identifiable by an URI (for example,
 * a local file or a web page)
 *
 * @since 4.5
 */
class KDE_EXPORT KActivityConsumer: public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentActivity READ currentActivity)
    Q_PROPERTY(QStringList availableActivities READ availableActivities)

public:
    explicit KActivityConsumer(QObject * parent = 0);

    ~KActivityConsumer();

    /**
     * @returns the id of the current activity
     */
    QString currentActivity() const;

    /**
     * @returns the list of all existing activities
     */
    QStringList availableActivities() const;

    /**
     * Returns the list of activities of a currently
     * registered resource. If back storage (Nepomuk)
     * doesn't exist, the result will contain only
     * the activities that were associated with the
     * specified resource in the current session.
     * @param uri uri of the resource
     */
    QStringList activitiesForResource(const KUrl & uri);

Q_SIGNALS:
    /**
     * This signal is emitted when the global
     * activity is changed
     * @param id id of the new current activity
     */
    void currentActivityChanged(const QString & id);

public Q_SLOTS:
    /**
     * Should be called when the client application
     * opens a new resource identifiable by an URI.
     * @param wid ID of the window that registers the resource
     * @param uri uri of the resource
     */
    void registerResourceWindow(WId wid, const KUrl & uri);

    /**
     * Should be called when the client application
     * closes a resource previously registered with
     * registerResourceWindow.
     * @param wid ID of the window that unregisters the resource
     * @param uri uri of the resource, if uri is not defined,
     * all resources associated with the window are unregistered
     */
    void unregisterResourceWindow(WId wid, const KUrl & uri = KUrl());

private:
    KActivityConsumerPrivate * const d;
};

#endif // ACTIVITY_CONSUMER_H
