/***************************************************************************
 *   applet.cpp                                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Sauer                                    *
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

#include "applet.h"
#include "jobwidget.h"
#include "notificationwidget.h"
#include "taskarea.h"

#include <QtGui/QApplication>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>

#include <KActionSelector>
#include <KConfigDialog>

#include <Solid/Device>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/framesvg.h>
#include <plasma/theme.h>

#include "../core/manager.h"
#include "../core/task.h"
#include "extendertask.h"


namespace SystemTray
{

K_EXPORT_PLASMA_APPLET(systemtray, Applet)


class Applet::Private
{
public:
    Private(Applet *q)
        : q(q),
          taskArea(0),
          configInterface(0),
	  notificationInterface(0),
          background(0),
          extenderTask(0)
    {
        if (!s_manager) {
            s_manager = new SystemTray::Manager();
        }

        ++s_managerUsage;
    }

    ~Private()
    {
        --s_managerUsage;
        if (s_managerUsage < 1) {
            delete s_manager;
            s_manager = 0;
            s_managerUsage = 0;
        }
    }

    void setTaskAreaGeometry();

    Applet *q;

    TaskArea *taskArea;
    QPointer<KActionSelector> configInterface;
    QPointer<QWidget> notificationInterface;
    QCheckBox *showNotifications;
    QCheckBox *showJobs;

    Plasma::FrameSvg *background;
    SystemTray::ExtenderTask *extenderTask;
    static SystemTray::Manager *s_manager;
    static int s_managerUsage;
};

Manager *Applet::Private::s_manager = 0;
int Applet::Private::s_managerUsage = 0;

Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      d(new Private(this))
{
    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/systemtray");
    d->background->setCacheAllRenderedFrames(true);
    d->taskArea = new TaskArea(this);

    setPopupIcon(QIcon());
    setPassivePopup(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
}

Applet::~Applet()
{
    //destroy any item in the systray, since it doesn't make sense for not detached notifications
    //and jobs to stay around after a plasma reboot
    foreach (Plasma::ExtenderItem *item, extender()->attachedItems()) {
        item->destroy();
    }

    delete d;
}

void Applet::init()
{
    KConfigGroup cg = config();
    QStringList hiddenTypes = cg.readEntry("hidden", QStringList());

    d->setTaskAreaGeometry();
    connect(Private::s_manager, SIGNAL(taskAdded(SystemTray::Task*)),
            d->taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(Private::s_manager, SIGNAL(taskChanged(SystemTray::Task*)),
            d->taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(Private::s_manager, SIGNAL(taskRemoved(SystemTray::Task*)),
            d->taskArea, SLOT(removeTask(SystemTray::Task*)));

    d->taskArea->setHiddenTypes(hiddenTypes);
    connect(d->taskArea, SIGNAL(sizeHintChanged(Qt::SizeHint)),
            this, SLOT(propogateSizeHintChange(Qt::SizeHint)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(checkSizes()));
    checkSizes();

    d->taskArea->syncTasks(Private::s_manager->tasks());

    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));

    KConfigGroup globalCg = globalConfig();
    if (globalCg.readEntry("ShowJobs", true)) {
        Private::s_manager->registerJobProtocol();
        connect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
    }

    if (globalCg.readEntry("ShowNotifications", true)) {
        Private::s_manager->registerNotificationProtocol();
        connect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    }
}

void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        bool vertical = formFactor() == Plasma::Vertical;

        if (!vertical) {
            policy.setVerticalPolicy(QSizePolicy::Expanding);
        } else {
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
        }

        setSizePolicy(policy);
        d->taskArea->setSizePolicy(policy);
        d->taskArea->setOrientation(vertical ? Qt::Vertical : Qt::Horizontal);
    }

    if (constraints & Plasma::SizeConstraint) {
        checkSizes();
    }
}


SystemTray::Manager *Applet::manager() const
{
    return d->s_manager;
}


void Applet::setGeometry(const QRectF &rect)
{
    Plasma::Applet::setGeometry(rect);

    if (d->taskArea) {
        d->setTaskAreaGeometry();
    }
}


void Applet::checkSizes()
{
    d->taskArea->checkSizes();

    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    d->background->getMargins(leftMargin, topMargin, rightMargin, bottomMargin);
    setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

    QSizeF preferredSize = d->taskArea->effectiveSizeHint(Qt::PreferredSize);
    preferredSize.setWidth(preferredSize.width() + leftMargin + rightMargin);
    preferredSize.setHeight(preferredSize.height() + topMargin + bottomMargin);
    setPreferredSize(preferredSize);

    QSizeF actualSize = size();
    Plasma::FormFactor f = formFactor();

    if (f == Plasma::Horizontal) {
        setMinimumSize(preferredSize.width(), 0);
    } else if (f == Plasma::Vertical) {
        setMinimumSize(0, preferredSize.height());
    } else if (actualSize.width() < preferredSize.width() ||
               actualSize.height() < preferredSize.height()) {
        setMinimumSize(22, 22);

        QSizeF constraint;
        if (actualSize.width() > actualSize.height()) {
            constraint = QSize(actualSize.width() - leftMargin - rightMargin, -1);
        } else {
            constraint = QSize(-1, actualSize.height() - topMargin - bottomMargin);
        }

        preferredSize = d->taskArea->effectiveSizeHint(Qt::PreferredSize, constraint);
        preferredSize.setWidth(qMax(actualSize.width(), preferredSize.width()));
        preferredSize.setHeight(qMax(actualSize.height(), preferredSize.height()));

        resize(preferredSize);
    }
}


void Applet::Private::setTaskAreaGeometry()
{
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    QRectF taskAreaRect(q->rect());
    taskAreaRect.moveLeft(leftMargin);
    taskAreaRect.moveTop(topMargin);
    taskAreaRect.setWidth(taskAreaRect.width() - leftMargin - rightMargin);
    taskAreaRect.setHeight(taskAreaRect.height() - topMargin - bottomMargin);

    taskArea->setGeometry(taskAreaRect);
}


void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

    QRect normalRect = rect().toRect();
    QRect lastRect(normalRect);
    d->background->setElementPrefix("lastelements");

    if (formFactor() == Plasma::Vertical) {
        const int rightEasement = d->taskArea->rightEasement() + d->background->marginSize(Plasma::BottomMargin);
        normalRect.setY(d->taskArea->leftEasement());
        normalRect.setBottom(normalRect.bottom() - rightEasement);

        lastRect.setY(normalRect.bottom() + 1);
        lastRect.setHeight(rightEasement);
    } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
        const int rightEasement = d->taskArea->rightEasement() + d->background->marginSize(Plasma::LeftMargin);
        normalRect.setWidth(normalRect.width() - d->taskArea->leftEasement());
        normalRect.setLeft(rightEasement);

        lastRect.setWidth(rightEasement);
    } else {
        const int rightEasement = d->taskArea->rightEasement() + d->background->marginSize(Plasma::RightMargin);
        normalRect.setX(d->taskArea->leftEasement());
        normalRect.setWidth(normalRect.width() - rightEasement);

        lastRect.setX(normalRect.right() + 1);
        lastRect.setWidth(rightEasement);
    }

    QRect r = normalRect.united(lastRect);

    painter->save();

    d->background->setElementPrefix(QString());
    d->background->resizeFrame(r.size());
    if (d->taskArea->rightEasement() > 0) {
        painter->setClipRect(normalRect);
    }
    d->background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));

    if (d->taskArea->rightEasement() > 0) {
        d->background->setElementPrefix("lastelements");
        d->background->resizeFrame(r.size());
        painter->setClipRect(lastRect);
        d->background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));
    }

    painter->restore();
}


void Applet::propogateSizeHintChange(Qt::SizeHint which)
{
    checkSizes();
    emit sizeHintChanged(which);
}


void Applet::createConfigurationInterface(KConfigDialog *parent)
{
    if (!d->configInterface) {
        d->configInterface = new KActionSelector();
        d->configInterface->setAvailableLabel(i18n("Visible icons:"));
        d->configInterface->setSelectedLabel(i18n("Hidden icons:"));
        d->configInterface->setShowUpDownButtons(false);

	KConfigGroup globalCg = globalConfig();
        d->notificationInterface = new QWidget();

	QLabel *description = new QLabel(i18n("Select the types of application feedback that should "
					"be integrated with the system tray:"), d->notificationInterface);
	description->setWordWrap(true);
	d->showJobs = new QCheckBox(i18n("Show Jobs"), d->notificationInterface);
	d->showJobs->setChecked(globalCg.readEntry("ShowJobs", true));
	d->showNotifications = new QCheckBox(i18n("Show Notifications"), d->notificationInterface);
	d->showNotifications->setChecked(globalCg.readEntry("ShowNotifications", true));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(description);
	layout->addWidget(d->showJobs);
	layout->addWidget(d->showNotifications);
	layout->addStretch();
	d->notificationInterface->setLayout(layout);

        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(d->configInterface, i18n("Auto Hide"));
	parent->addPage(d->notificationInterface, i18n("Notifications"));
    }

    QListWidget *visibleList = d->configInterface->availableListWidget();
    QListWidget *hiddenList = d->configInterface->selectedListWidget();

    visibleList->clear();
    hiddenList->clear();

    foreach (Task *task, Private::s_manager->tasks()) {
        if (!task->isHideable()) {
            continue;
        }

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setText(task->name());
        listItem->setIcon(task->icon());
        listItem->setData(Qt::UserRole, task->typeId());

        if (d->taskArea->isHiddenType(task->typeId(), false)) {
            hiddenList->addItem(listItem);
        } else {
            visibleList->addItem(listItem);
        }
    }
}


void Applet::configAccepted()
{
    QStringList hiddenTypes;

    QListWidget *hiddenList = d->configInterface->selectedListWidget();
    for (int i = 0; i < hiddenList->count(); ++i) {
        hiddenTypes << hiddenList->item(i)->data(Qt::UserRole).toString();
    }

    d->taskArea->setHiddenTypes(hiddenTypes);
    d->taskArea->syncTasks(Private::s_manager->tasks());

    KConfigGroup cg = config();
    cg.writeEntry("hidden", hiddenTypes);
   
    KConfigGroup globalCg = globalConfig();
    globalCg.writeEntry("ShowJobs", d->showJobs->isChecked());
    globalCg.writeEntry("ShowNotifications", d->showNotifications->isChecked());

    if (d->showJobs->isChecked()) {
        Private::s_manager->registerJobProtocol();
        connect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
    } else {
	Private::s_manager->unregisterJobProtocol();
        disconnect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
    }

    if (d->showNotifications->isChecked()) {
        Private::s_manager->registerNotificationProtocol();
        connect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    } else {
	Private::s_manager->unregisterNotificationProtocol();
        disconnect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    }

    emit configNeedsSaving();
}


void Applet::addNotification(Notification *notification)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "notification");
    extenderItem->setWidget(new NotificationWidget(notification, extenderItem));

    connect(extenderItem, SIGNAL(destroyed()), this, SLOT(hidePopupIfEmpty()));

    showPopup();
}

void Applet::addJob(Job *job)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "job");
    extenderItem->setWidget(new JobWidget(job, extenderItem));

    connect(extenderItem, SIGNAL(destroyed()), this, SLOT(hidePopupIfEmpty()));

    showPopup(5000);
}

void Applet::initExtenderItem(Plasma::ExtenderItem *extenderItem)
{
    if (extenderItem->config().readEntry("type", "") == "notification") {
        extenderItem->setWidget(new NotificationWidget(0, extenderItem));
    } else {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    }
}

void Applet::hidePopupIfEmpty()
{
    if (d->extenderTask && extender()->attachedItems().isEmpty()) {
        hidePopup();

        // even though there is code to do this in popupEvent... we may need to delete
        // here anyways because the popup may already be hidden, resultin gin no
        // popupEvent
        delete d->extenderTask;
        d->extenderTask = 0;
    }
}

void Applet::popupEvent(bool visibility)
{
    kDebug() << visibility << extender()->attachedItems().isEmpty();
    if (extender()->attachedItems().isEmpty()) {
        delete d->extenderTask;
        d->extenderTask = 0;
    } else {
        if (!d->extenderTask) {
            d->extenderTask = new SystemTray::ExtenderTask(this);
            d->extenderTask->setIcon("help-about");
        }

        Private::s_manager->addTask(d->extenderTask);
    }
}


}

#include "applet.moc"
