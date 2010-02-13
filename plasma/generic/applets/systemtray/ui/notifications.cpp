/***************************************************************************
 *   applet.cpp                                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Sauer                                    *
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

#include "notifications.h"

//FIXME: this has to go ASAP
#define NOTIFICATIONAPPLET

#include <QtGui/QApplication>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>
#include <QtCore/QProcess>


#include <KConfigDialog>
#include <KComboBox>
#include <KWindowSystem>

#include <Solid/Device>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/theme.h>
#include <plasma/dataenginemanager.h>
#include <plasma/dataengine.h>
#include <Plasma/TabBar>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>

#include "config.h"
#ifdef HAVE_LIBXSS      // Idle detection.
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <fixx11h.h>
#endif // HAVE_LIBXSS

#include "../core/notificationsmanager.h"
#include "../core/notification.h"
#include "../core/task.h"
#include "../core/extendertask.h"
#include "../core/completedjobnotification.h"
#include "jobwidget.h"
#include "jobtotalswidget.h"
#include "notificationscroller.h"
#include "notificationstack.h"
#include "stackdialog.h"

namespace SystemTray
{


K_EXPORT_PLASMA_APPLET(notifications, Notifications)


Manager *Notifications::s_manager = 0;
int Notifications::s_managerUsage = 0;

Notifications::Notifications(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      m_jobSummaryWidget(0),
      m_notificationStack(0),
      m_notificationStackDialog(0),
      m_standaloneJobSummaryWidget(0),
      m_standaloneJobSummaryDialog(0)
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
    }

    ++s_managerUsage;

    setPopupIcon(QIcon());
    setPassivePopup(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
}

Notifications::~Notifications()
{
    // stop listening to the manager
    disconnect(s_manager, 0, this, 0);

    foreach (Notification *notification, s_manager->notifications()) {
        // we don't want a destroyed managed after the destruction of manager
        disconnect(notification, 0, this, 0);
    }

    --s_managerUsage;
    if (s_managerUsage < 1) {
        delete s_manager;
        s_manager = 0;
        s_managerUsage = 0;
    }
    delete m_notificationStackDialog;
}

void Notifications::init()
{
    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));
    extender()->setWindowFlags(Qt::X11BypassWindowManagerHint);

    m_busyWidget = new ExtenderTaskBusyWidget(this, s_manager);
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(this);
    lay->addItem(m_busyWidget);

    configChanged();
}

void Notifications::configChanged()
{
    KConfigGroup cg = config();

    if (cg.readEntry("AutoHidePopup", true)) {
        m_autoHideTimeout = 6000;
    } else {
        m_autoHideTimeout = 0;
    }

    KConfigGroup globalCg = globalConfig();
    bool createExtenderTask = false;
    if (globalCg.readEntry("ShowJobs", true)) {
        createExtenderTask = true;
        createJobGroups();

        s_manager->registerJobProtocol();
        connect(s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)), Qt::UniqueConnection);
        connect(s_manager, SIGNAL(jobRemoved(SystemTray::Job*)),
                this, SLOT(finishJob(SystemTray::Job*)), Qt::UniqueConnection);
    } else {
        s_manager->unregisterJobProtocol();
        disconnect(s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                   this, SLOT(addJob(SystemTray::Job*)));
        disconnect(s_manager, SIGNAL(jobRemoved(SystemTray::Job*)),
                   this, SLOT(finishJob(SystemTray::Job*)));
    }

    if (globalCg.readEntry("ShowNotifications", true)) {
        createExtenderTask = true;
        s_manager->registerNotificationProtocol();
        connect(s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)), Qt::UniqueConnection);
    } else {
        s_manager->unregisterNotificationProtocol();
        disconnect(s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                   this, SLOT(addNotification(SystemTray::Notification*)));
    }
}

void Notifications::syncNotificationBarNeeded()
{
    if (!s_manager) {
        return;
    }

    if (s_manager->notifications().count() > 0) {
        if (!extender()->item("notifications")) {
            Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
            extenderItem->config().writeEntry("type", "notification");
            extenderItem->setName("notifications");
            extenderItem->setTitle(i18n("Notifications"));
            extenderItem->setIcon("dialog-information");
            extenderItem->showCloseButton();

            m_notificationScroller = new NotificationScroller(extenderItem);
            connect(m_notificationScroller, SIGNAL(scrollerEmpty()), extenderItem, SLOT(destroy()));
            extenderItem->setWidget(m_notificationScroller);
            extenderItem->setExtender(extender());
        }
    } else if (extender()->item("notifications")) {
        //don't let him in the config file
        extender()->item("notifications")->destroy();
    }
}


SystemTray::Manager *Notifications::manager() const
{
    return s_manager;
}


void Notifications::createConfigurationInterface(KConfigDialog *parent)
{
    if (!m_notificationInterface) {
        KConfigGroup globalCg = globalConfig();
        m_notificationInterface = new QWidget();

        m_notificationUi.setupUi(m_notificationInterface.data());

        m_notificationUi.showJobs->setChecked(globalCg.readEntry("ShowJobs", true));
        m_notificationUi.showNotifications->setChecked(globalCg.readEntry("ShowNotifications", true));

        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(m_notificationInterface.data(), i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));
    }
}

void Notifications::configAccepted()
{
    //TODO put in a single page
    //cg.writeEntry("AutoHidePopup", m_autoHideUi.autoHide->isChecked());

    KConfigGroup globalCg = globalConfig();
    globalCg.writeEntry("ShowJobs", m_notificationUi.showJobs->isChecked());
    globalCg.writeEntry("ShowNotifications", m_notificationUi.showNotifications->isChecked());

    emit configNeedsSaving();
}

void Notifications::addNotification(Notification *notification)
{
    syncNotificationBarNeeded();

    //At this point we are sure the pointer is valid
    m_notificationScroller->addNotification(notification);

    if (isPopupShowing()) {
        return;
    }

    if (!m_notificationStack) {
        m_notificationStack = new NotificationStack(this);
        if (containment() && containment()->corona()) {
            containment()->corona()->addOffscreenWidget(m_notificationStack);
        }
        m_notificationStackDialog = new StackDialog;
        m_notificationStackDialog->setApplet(this);
        m_notificationStackDialog->setNotificationStack(m_notificationStack);
        connect(m_notificationStack, SIGNAL(stackEmpty()), m_notificationStackDialog, SLOT(hide()));

        if (m_standaloneJobSummaryDialog) {
            m_notificationStackDialog->setWindowToTile(m_standaloneJobSummaryDialog);
        }
    }


    m_notificationStack->addNotification(notification);
    m_notificationStackDialog->syncToGraphicsWidget();

    if (containment() && containment()->corona()) {
        m_notificationStackDialog->move(containment()->corona()->popupPosition(this, m_notificationStackDialog->size()));

        if (!m_notificationStackDialog->isVisible()) {
            m_notificationStack->setCurrentNotification(notification);
        }

        m_notificationStackDialog->show();
        Plasma::WindowEffects::slideWindow(m_notificationStackDialog, location());
    }
}

void Notifications::addJob(Job *job)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "job");
    extenderItem->setWidget(new JobWidget(job, extenderItem));

    extenderItem->setGroup(extender()->group("jobGroup"));

    //show the tiny standalone overview
    if (!m_standaloneJobSummaryWidget) {
        m_standaloneJobSummaryDialog = new Plasma::Dialog();
        if (m_notificationStackDialog) {
            m_notificationStackDialog->setWindowToTile(m_standaloneJobSummaryDialog);
        }

        KWindowSystem::setOnAllDesktops(m_standaloneJobSummaryDialog->winId(), true);

        m_standaloneJobSummaryWidget = new JobTotalsWidget(s_manager->jobTotals(), this);
        if (containment() && containment()->corona()) {
            containment()->corona()->addOffscreenWidget(m_standaloneJobSummaryWidget);
        }
        m_standaloneJobSummaryDialog->setGraphicsWidget(m_standaloneJobSummaryWidget);
        //FIXME:sizing hack and layout issues..
        m_standaloneJobSummaryWidget->resize(m_standaloneJobSummaryWidget->size().width(), 32);
    }

    m_standaloneJobSummaryDialog->syncToGraphicsWidget();

    if (containment() && containment()->corona()) {
        m_standaloneJobSummaryDialog->move(containment()->corona()->popupPosition(this, m_standaloneJobSummaryDialog->size()));
        m_standaloneJobSummaryDialog->show();
        KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::SkipTaskbar|NET::SkipPager);
        KWindowSystem::raiseWindow(m_standaloneJobSummaryDialog->winId());
        Plasma::WindowEffects::slideWindow(m_standaloneJobSummaryDialog, location());
    }
}

void Notifications::initExtenderItem(Plasma::ExtenderItem *extenderItem)
{
    if (extenderItem->name() == "jobGroup") {
        m_jobSummaryWidget = new JobTotalsWidget(s_manager->jobTotals(), extenderItem);
        extenderItem->setWidget(m_jobSummaryWidget);
        return;
    }

    if (extenderItem->config().readEntry("type", "") == "job") {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    //unknown type, this should never happen
    } else {
        extenderItem->destroy();
    }

}

void Notifications::popupEvent(bool show)
{
    //decide about showing the tiny progressbar or not
    if (m_standaloneJobSummaryDialog) {
        if (show || !s_manager->jobs().isEmpty()) {
            m_standaloneJobSummaryDialog->setVisible(!show);
            if (!show) {
                KWindowSystem::raiseWindow(m_standaloneJobSummaryDialog->winId());
                KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::SkipTaskbar|NET::SkipPager);
            }
        }
    }

    if (m_notificationStackDialog && show) {
        m_notificationStackDialog->hide();
    }

    Plasma::ExtenderGroup * jobGroup = extender()->group("jobGroup");
    if (!jobGroup) {
        return;
    }

    foreach (Plasma::ExtenderItem *item, jobGroup->items()) {
        JobWidget *job = dynamic_cast<JobWidget *>(item->widget());
        if (job) {
            job->poppedUp(show);
        }
    }
}

void Notifications::finishJob(SystemTray::Job *job)
{
    //finished all jobs? hide the mini progressbar
    if (m_standaloneJobSummaryDialog && s_manager->jobs().isEmpty()) {
        m_standaloneJobSummaryDialog->hide();
    }

    //create a fake notification
    CompletedJobNotification *notification = new CompletedJobNotification(this);
    notification->setJob(job);
    s_manager->addNotification(notification);
}

void Notifications::open(const QString &url)
{
    //kDebug() << "open " << url;
    QProcess::startDetached("kde-open", QStringList() << url);
}

void Notifications::createJobGroups()
{
    if (!extender()->hasItem("jobGroup")) {
        Plasma::ExtenderGroup *extenderGroup = new Plasma::ExtenderGroup(extender());
        extenderGroup->setName("jobGroup");
        initExtenderItem(extenderGroup);
    }
}

}

#include "notifications.moc"