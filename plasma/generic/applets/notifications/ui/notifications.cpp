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
#include <KIconLoader>

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
#include <Plasma/Animator>
#include <Plasma/Animation>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>

#include "config-notifications.h"
#ifdef HAVE_LIBXSS      // Idle detection.
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <fixx11h.h>
#endif // HAVE_LIBXSS

#include "../core/notificationsmanager.h"
#include "../core/notification.h"
#include "../core/completedjobnotification.h"
#include "busywidget.h"
#include "jobwidget.h"
#include "jobtotalswidget.h"
#include "notificationgroup.h"
#include "notificationstack.h"
#include "stackdialog.h"


K_EXPORT_PLASMA_APPLET(notifications, Notifications)


Notifications::Notifications(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      m_jobSummaryWidget(0),
      m_autoHidePopup(true),
      m_notificationStack(0),
      m_notificationStackDialog(0),
      m_standaloneJobSummaryWidget(0),
      m_standaloneJobSummaryDialog(0),
      m_busyWidget(0)
{
    m_manager = new Manager(this);

    setPopupIcon(QIcon());
    setPassivePopup(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
    setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
}

Notifications::~Notifications()
{
    // stop listening to the manager
    disconnect(m_manager, 0, this, 0);
    if (m_notificationStackDialog) {
        disconnect(m_notificationStackDialog, 0, this, 0);
    }

    foreach (Notification *notification, m_manager->notifications()) {
        // we don't want a destroyed managed after the destruction of manager
        disconnect(notification, 0, this, 0);
    }

    //has to be deleted before the manager because it will access it
    delete m_busyWidget;
    delete m_notificationStackDialog;
    delete m_standaloneJobSummaryDialog;
}

void Notifications::init()
{
    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));

    m_busyWidget = new BusyWidget(this, m_manager);
    connect(m_busyWidget, SIGNAL(clicked()), this, SLOT(togglePopup()));
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(this);
    setContentsMargins(0, 0, 0, 0);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addItem(m_busyWidget);

    configChanged();
    setStatus(Plasma::PassiveStatus);
}

void Notifications::configChanged()
{
    KConfigGroup cg = config();

    m_autoHidePopup = cg.readEntry("AutoHidePopup", true);
    if (m_notificationStackDialog) {
        m_notificationStackDialog->setAutoHide(m_autoHidePopup);
    }

    if (cg.readEntry("ShowJobs", true)) {
        createJobGroups();

        m_manager->registerJobProtocol();
        connect(m_manager, SIGNAL(jobAdded(Job*)),
                this, SLOT(addJob(Job*)), Qt::UniqueConnection);
        connect(m_manager, SIGNAL(jobRemoved(Job*)),
                this, SLOT(finishJob(Job*)), Qt::UniqueConnection);
    } else {
        delete extender()->group("jobGroup");
        m_manager->unregisterJobProtocol();
        disconnect(m_manager, SIGNAL(jobAdded(Job*)),
                   this, SLOT(addJob(Job*)));
        disconnect(m_manager, SIGNAL(jobRemoved(Job*)),
                   this, SLOT(finishJob(Job*)));
    }

    if (cg.readEntry("ShowNotifications", true)) {
        m_manager->registerNotificationProtocol();
        connect(m_manager, SIGNAL(notificationAdded(Notification*)),
                this, SLOT(addNotification(Notification*)), Qt::UniqueConnection);
    } else {
        m_manager->unregisterNotificationProtocol();
        disconnect(m_manager, SIGNAL(notificationAdded(Notification*)),
                   this, SLOT(addNotification(Notification*)));
    }
}

void Notifications::syncNotificationBarNeeded()
{
    if (!m_manager) {
        return;
    }

    if (m_manager->notifications().isEmpty()) {
        if (extender()->item("notifications")) {
            //don't let him in the config file
            extender()->item("notifications")->destroy();
        }
    } else if (!extender()->item("notifications")) {
        m_notificationGroup = new NotificationGroup(extender());
    }
}

Manager *Notifications::manager() const
{
    return m_manager;
}

void Notifications::createConfigurationInterface(KConfigDialog *parent)
{
    if (!m_notificationInterface) {
        KConfigGroup cg = config();
        m_notificationInterface = new QWidget();

        m_notificationUi.setupUi(m_notificationInterface.data());

        m_notificationUi.showJobs->setChecked(cg.readEntry("ShowJobs", true));
        m_notificationUi.showNotifications->setChecked(cg.readEntry("ShowNotifications", true));

        m_notificationUi.autoHide->setChecked(config().readEntry("AutoHidePopup", true));

        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(m_notificationInterface.data(), i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));

        connect(m_notificationUi.showNotifications, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
        connect(m_notificationUi.showJobs, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
        connect(m_notificationUi.autoHide, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    }
}

void Notifications::configAccepted()
{
    //TODO put in a single page
    //cg.writeEntry("AutoHidePopup", m_autoHideUi.autoHide->isChecked());

    KConfigGroup cg = config();
    cg.writeEntry("ShowJobs", m_notificationUi.showJobs->isChecked());
    cg.writeEntry("ShowNotifications", m_notificationUi.showNotifications->isChecked());
    cg.writeEntry("AutoHidePopup", m_notificationUi.autoHide->isChecked());

    emit configNeedsSaving();
}

void Notifications::addNotification(Notification *notification)
{
    syncNotificationBarNeeded();

    //At this point we are sure the pointer is valid
    m_notificationGroup.data()->addNotification(notification);


    if (isPopupShowing()) {
        return;
    }

    if (!m_notificationStack) {
        m_notificationStack = new NotificationStack(this);
        if (containment() && containment()->corona()) {
            containment()->corona()->addOffscreenWidget(m_notificationStack);
        }
        m_notificationStackDialog = new StackDialog;
        m_notificationStackDialog->setNotificationStack(m_notificationStack);
        m_notificationStackDialog->setApplet(this);
        connect(m_notificationStack, SIGNAL(stackEmpty()), m_notificationStackDialog, SLOT(hide()));
        connect(m_notificationStack, SIGNAL(showRequested()), m_notificationStackDialog, SLOT(show()));
        m_notificationStackDialog->setAutoHide(m_autoHidePopup);

        if (m_standaloneJobSummaryDialog) {
            m_notificationStackDialog->setWindowToTile(m_standaloneJobSummaryDialog);
        }
    }


    m_notificationStack->addNotification(notification);
    m_notificationStackDialog->syncToGraphicsWidget();

    if (containment() && containment()->corona()) {
        if (!m_notificationStackDialog->isVisible()) {
            m_notificationStack->setCurrentNotification(notification);
        }

        KWindowSystem::setOnAllDesktops(m_notificationStackDialog->winId(), true);
        m_notificationStackDialog->show();
    }

    Plasma::Animation *pulse = Plasma::Animator::create(Plasma::Animator::PulseAnimation, m_busyWidget);
    pulse->setTargetWidget(m_busyWidget);
    pulse->start(QAbstractAnimation::DeleteWhenStopped);
}

void Notifications::addJob(Job *job)
{
    Plasma::ExtenderGroup *group = extender()->group("jobGroup");

    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->setTransient(true);
    extenderItem->config().writeEntry("type", "job");
    extenderItem->setWidget(new JobWidget(job, extenderItem));

    extenderItem->setGroup(group);

    if (group) {
        group->setCollapsed(group->items().count() < 2);
    }

    if (isPopupShowing()) {
        return;
    }

    //show the tiny standalone overview
    if (!m_standaloneJobSummaryWidget) {
        m_standaloneJobSummaryDialog = new Plasma::Dialog();
        KWindowSystem::setType(m_standaloneJobSummaryDialog->winId(), NET::Dock);
        if (m_notificationStackDialog) {
            m_notificationStackDialog->setWindowToTile(m_standaloneJobSummaryDialog);
        }

        m_standaloneJobSummaryWidget = new JobTotalsWidget(m_manager->jobTotals(), this);
        if (containment() && containment()->corona()) {
            containment()->corona()->addOffscreenWidget(m_standaloneJobSummaryWidget);
        }
        m_standaloneJobSummaryDialog->setGraphicsWidget(m_standaloneJobSummaryWidget);
        //FIXME:sizing hack and layout issues..
        m_standaloneJobSummaryWidget->resize(m_standaloneJobSummaryWidget->size().width(), 32);
        m_standaloneJobSummaryWidget->setMaximumHeight(32);
        m_standaloneJobSummaryWidget->setMinimumHeight(32);
        m_standaloneJobSummaryWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    m_standaloneJobSummaryDialog->syncToGraphicsWidget();
    KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::KeepBelow);

    if (containment() && containment()->corona()) {
        m_standaloneJobSummaryDialog->move(containment()->corona()->popupPosition(this, m_standaloneJobSummaryDialog->size()));
        m_standaloneJobSummaryDialog->show();
        Plasma::WindowEffects::slideWindow(m_standaloneJobSummaryDialog, location());

        KWindowSystem::setOnAllDesktops(m_standaloneJobSummaryDialog->winId(), true);
        KWindowSystem::clearState(m_standaloneJobSummaryDialog->winId(), NET::KeepAbove|NET::StaysOnTop);

        KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::SkipTaskbar|NET::SkipPager);
        KWindowSystem::raiseWindow(m_standaloneJobSummaryDialog->winId());
        KWindowSystem::setOnAllDesktops(m_standaloneJobSummaryDialog->winId(), true);
    }
}

void Notifications::initExtenderItem(Plasma::ExtenderItem *extenderItem)
{
    if (extenderItem->name() == "jobGroup") {
        m_jobSummaryWidget = new JobTotalsWidget(m_manager->jobTotals(), extenderItem);
        extenderItem->setWidget(m_jobSummaryWidget);
        Plasma::ExtenderGroup *group = qobject_cast<Plasma::ExtenderGroup*>(extenderItem);
        if (group) {
            extenderItem->setCollapsed(!group->isGroupCollapsed());
        }
        return;
    }

    if (extenderItem->config().readEntry("type", QString()) == "job") {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    } else {
        //unknown type, this should never happen
        extenderItem->destroy();
    }

}

void Notifications::popupEvent(bool show)
{
    if (m_busyWidget) {
        m_busyWidget->suppressToolTips(show);
    }

    //decide about showing the tiny progressbar or not
    if (m_standaloneJobSummaryDialog) {
        if (show || !m_manager->jobs().isEmpty()) {
            if (!show) {
                KWindowSystem::raiseWindow(m_standaloneJobSummaryDialog->winId());
                KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::SkipTaskbar|NET::SkipPager);
                KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::KeepBelow);
            } else {
                m_standaloneJobSummaryDialog->hide();
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

void Notifications::finishJob(Job *job)
{
    //finished all jobs? hide the mini progressbar
    if (m_standaloneJobSummaryDialog && m_manager->jobs().isEmpty()) {
        m_standaloneJobSummaryDialog->hide();
    }

    //create a fake notification
    CompletedJobNotification *notification = new CompletedJobNotification(this);
    notification->setJob(job);
    m_manager->addNotification(notification);

    Plasma::ExtenderGroup *group = extender()->group("jobGroup");
    if (group) {
        // < 3 because the second still hasn't been removed from the extendergroup
        group->setCollapsed(!group->isGroupCollapsed() && group->items().count() < 3);
    }
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
        extenderGroup->setAutoHide(true);
    } else if (extender()->group("jobGroup")) {
        extender()->group("jobGroup")->setAutoHide(true);
    }
}


#include "notifications.moc"
