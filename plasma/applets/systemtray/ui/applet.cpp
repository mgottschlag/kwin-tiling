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
#include <QtGui/QIcon>
#include <QtGui/QListWidget>

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
          background(0),
          extenderTask(0)
    {
    }

    void setTaskAreaGeometry();

    Applet *q;

    TaskArea *taskArea;
    QPointer<KActionSelector> configInterface;

    Plasma::FrameSvg *background;
    SystemTray::ExtenderTask *extenderTask;
};


Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      d(new Private(this))
{
    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/systemtray");

    setPopupIcon(QIcon());
    setAspectRatioMode(Plasma::KeepAspectRatio);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
}

Applet::~Applet()
{
    delete d;
}

void Applet::init()
{
    KConfigGroup cg = config();
    QStringList hiddenTypes = cg.readEntry("hidden", QStringList());

    d->taskArea = new TaskArea(this);
    d->setTaskAreaGeometry();
    connect(Manager::self(), SIGNAL(taskAdded(SystemTray::Task*)),
            d->taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(Manager::self(), SIGNAL(taskChanged(SystemTray::Task*)),
            d->taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(Manager::self(), SIGNAL(taskRemoved(SystemTray::Task*)),
            d->taskArea, SLOT(removeTask(SystemTray::Task*)));

    d->taskArea->setHiddenTypes(hiddenTypes);
    connect(d->taskArea, SIGNAL(sizeHintChanged(Qt::SizeHint)),
            this, SLOT(propogateSizeHintChange(Qt::SizeHint)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(checkSizes()));
    checkSizes();

    d->taskArea->syncTasks(Manager::self()->tasks());

    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));
    connect(SystemTray::Manager::self(), SIGNAL(notificationAdded(SystemTray::Notification*)),
            this, SLOT(addNotification(SystemTray::Notification*)));
    connect(SystemTray::Manager::self(), SIGNAL(jobAdded(SystemTray::Job*)),
            this, SLOT(addJob(SystemTray::Job*)));

}


void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        policy.setHeightForWidth(true);
        bool horizontal = formFactor() != Plasma::Vertical;

        if (horizontal) {
            policy.setVerticalPolicy(QSizePolicy::Expanding);
        } else {
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
        }

        setSizePolicy(policy);
        d->taskArea->setSizePolicy(policy);
        d->taskArea->setOrientation(horizontal ? Qt::Horizontal : Qt::Vertical);
    }

    if (constraints & Plasma::SizeConstraint) {
        checkSizes();
    }
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
    if (formFactor() == Plasma::Planar && (actualSize.width() < preferredSize.width() ||
                                           actualSize.height() < preferredSize.height())) {

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

    QRect r = rect().toRect();

    if (formFactor() == Plasma::Vertical) {
        r.setY(d->taskArea->easement());
    } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
        r.setWidth(r.width() - d->taskArea->easement());
    } else {
        r.setX(d->taskArea->easement());
        kDebug() << "seting easement to" << d->taskArea->easement() << r << size();
    }

    d->background->resizeFrame(r.size());
    d->background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));
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

        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->setMainWidget(d->configInterface);
    }

    QListWidget *visibleList = d->configInterface->availableListWidget();
    QListWidget *hiddenList = d->configInterface->selectedListWidget();

    visibleList->clear();
    hiddenList->clear();

    foreach (Task *task, Manager::self()->tasks()) {
        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setText(task->name());
        listItem->setIcon(task->icon());
        listItem->setData(Qt::UserRole, task->typeId());

        if (d->taskArea->isHiddenType(task->typeId())) {
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
    d->taskArea->syncTasks(Manager::self()->tasks());

    KConfigGroup cg = config();
    cg.writeEntry("hidden", hiddenTypes);
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
            if (Solid::Device::listFromType(Solid::DeviceInterface::Battery).isEmpty()) {
                d->extenderTask->setIcon("computer");
            } else {
                d->extenderTask->setIcon("computer-laptop");
            }
        }

        Manager::self()->addTask(d->extenderTask);
    }
}


}

#include "applet.moc"
