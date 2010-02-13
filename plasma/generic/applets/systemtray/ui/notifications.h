/***************************************************************************
 *   applet.h                                                              *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <plasma/popupapplet.h>

#include "ui_protocols.h"


namespace Plasma
{
class ExtenderItem;
class TabBar;
class Dialog;
}

class NotificationWidget;
class StackDialog;

namespace SystemTray
{

class Job;
class JobTotalsWidget;
class Manager;
class Notification;
class NotificationScroller;
class NotificationStack;

class Notifications : public Plasma::PopupApplet
{
    Q_OBJECT

public:
    explicit Notifications(QObject *parent, const QVariantList &arguments = QVariantList());
    ~Notifications();

    void init();
    Manager *manager() const;

protected:
    void createConfigurationInterface(KConfigDialog *parent);
    void initExtenderItem(Plasma::ExtenderItem *extenderItem);
    void configChanged();

    void popupEvent(bool show);

private slots:
    void configAccepted();
    void addNotification(SystemTray::Notification *notification);
    void addJob(SystemTray::Job *job);
    void finishJob(SystemTray::Job *job);
    void open(const QString &url);
    void syncNotificationBarNeeded();

private:
    void createJobGroups();
    void initExtenderTask(bool create);

    static SystemTray::Manager *s_manager;
    static int s_managerUsage;

    QWeakPointer<QWidget> m_notificationInterface;
    QDateTime m_lastActivity;

    JobTotalsWidget *m_jobSummaryWidget;
    int m_autoHideTimeout;

    NotificationScroller *m_notificationScroller;
    NotificationStack *m_notificationStack;
    StackDialog *m_notificationStackDialog;
    JobTotalsWidget *m_standaloneJobSummaryWidget;
    Plasma::Dialog *m_standaloneJobSummaryDialog;

    Ui::ProtocolsConfig m_notificationUi;
};

}


#endif
