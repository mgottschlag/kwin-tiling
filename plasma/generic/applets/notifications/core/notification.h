/***************************************************************************
 *   notification.h                                                        *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QImage>
#include <QtCore/QHash>
#include <QtCore/QObject>

#include <QtGui/QIcon>



class Notification : public QObject
{
    Q_OBJECT

public:
    Notification(QObject *parent = 0);
    virtual ~Notification();

    QString applicationName() const;
    QIcon applicationIcon() const;
    QString message() const;
    QString summary() const;
    int timeout() const;
    QImage image() const;

    void setUrgency(int urgency);
    int urgency() const;

    QHash<QString, QString> actions() const;
    QStringList actionOrder() const;

    bool isExpired() const;

    void setRead(const bool read);
    bool isRead() const;

    void setDeleteTimeout(const int time);
    int deleteTimeOut() const;

public slots:
    virtual void triggerAction(const QString &actionId);
    virtual void remove();
    virtual void linkActivated(const QString &link);
    void startDeletionCountdown();
    void hide();
    void destroy();

signals:
    void changed(Notification *notification = 0);

    /**
     * Emitted when the notification is about to be destroyed
     **/
    void notificationDestroyed(Notification *notification = 0);

    /**
     * emitted when the notification wants to hide itself
     */
    void expired(Notification *notification = 0);

protected:
    void setApplicationName(const QString &applicationName);
    void setApplicationIcon(const QIcon &applicationIcon);
    void setMessage(const QString &message);
    void setSummary(const QString &summary);
    void setImage(QImage image);
    void setTimeout(int timeout);
    void setActions(const QHash<QString, QString> &actions);
    void setActionOrder(const QStringList &actionOrder);

private:
    class Private;
    Private* const d;
};



#endif
