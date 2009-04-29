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

#include <QtGui/QApplication>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtCore/QProcess>

#include <KActionSelector>
#include <KConfigDialog>

#include <Solid/Device>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/theme.h>

#include "../core/manager.h"
#include "extendertask.h"
#include "jobwidget.h"
#include "jobtotalswidget.h"
#include "notificationwidget.h"
#include "taskarea.h"

#include "ui_protocols.h"

namespace SystemTray
{

static const int autoHideTimeout = 6000;

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
          jobSummaryWidget(0),
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
    QList<Job*> jobs;
    QSet<Task::Category> shownCategories;

    Plasma::FrameSvg *background;
    JobTotalsWidget *jobSummaryWidget;
    QPointer<SystemTray::ExtenderTask> extenderTask;
    static SystemTray::Manager *s_manager;
    static int s_managerUsage;

    Ui::ProtocolsConfig ui;
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

    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));

    KConfigGroup globalCg = globalConfig();

    if (globalCg.readEntry("ShowApplicationStatus", true)) {
        d->shownCategories.insert(Task::ApplicationStatus);
    }
    if (globalCg.readEntry("ShowCommunications", true)) {
        d->shownCategories.insert(Task::Communications);
    }
    if (globalCg.readEntry("ShowSystemServices", true)) {
        d->shownCategories.insert(Task::SystemServices);
    }
    if (globalCg.readEntry("ShowHardware", true)) {
        d->shownCategories.insert(Task::Hardware);
    }

    d->shownCategories.insert(Task::UnknownCategory);

    //destroy any item in the systray, that doesn't belong to the completedJobsGroup, since running
    //jobs and notifications can't really survive reboots anyways
    foreach (Plasma::ExtenderItem *item, extender()->attachedItems()) {
        if (!item->isGroup() && (item->group() != extender()->group("completedJobsGroup"))) {
            item->destroy();
        }
    }

    if (globalCg.readEntry("ShowJobs", true)) {
        createJobGroups();

        Private::s_manager->registerJobProtocol();
        connect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
        connect(Private::s_manager, SIGNAL(jobRemoved(SystemTray::Job*)),
                this, SLOT(finishJob(SystemTray::Job*)));

        //there might still be completed jobs in the tray, in which case we directly want to show
        //the extender task spinner.
        if (extender()->group("completedJobsGroup") &&
            !extender()->group("completedJobsGroup")->items().isEmpty()) {
            if (!d->extenderTask) {
                d->extenderTask = new SystemTray::ExtenderTask(this, Private::s_manager, extender());
                d->extenderTask->setIcon("help-about");
            }
            Private::s_manager->addTask(d->extenderTask);
        }
    }

    if (globalCg.readEntry("ShowNotifications", true)) {
        Private::s_manager->registerNotificationProtocol();
        connect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    }

    d->taskArea->syncTasks(Private::s_manager->tasks());
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

QSet<Task::Category> Applet::shownCategories() const
{
    return d->shownCategories;
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

    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    if (f == Plasma::Horizontal) {
        setMinimumSize(preferredSize.width(), 0);
        setMaximumSize(preferredSize.width(), QWIDGETSIZE_MAX);
    } else if (f == Plasma::Vertical) {
        setMinimumSize(0, preferredSize.height());
        setMaximumSize(QWIDGETSIZE_MAX, preferredSize.height());
    } else if (actualSize.width() < preferredSize.width() ||
               actualSize.height() < preferredSize.height()) {
        setMinimumSize(22, 22);

        QSizeF constraint;
        if (actualSize.width() > actualSize.height()) {
            constraint = QSize(actualSize.width() - leftMargin - rightMargin, -1);
        } else {
            constraint = QSize(-1, actualSize.height() - topMargin - bottomMargin);
        }

        preferredSize = d->taskArea->effectiveSizeHint(Qt::PreferredSize, actualSize);
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

        lastRect.setX(0);
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

        d->ui.setupUi(d->notificationInterface);

        d->ui.showJobs->setChecked(globalCg.readEntry("ShowJobs", true));
        d->ui.showNotifications->setChecked(globalCg.readEntry("ShowNotifications", true));

        d->ui.showApplicationStatus->setChecked(globalCg.readEntry("ShowApplicationStatus", true));
        d->ui.showCommunications->setChecked(globalCg.readEntry("ShowCommunications", true));
        d->ui.showSystemServices->setChecked(globalCg.readEntry("ShowSystemServices", true));
        d->ui.showHardware->setChecked(globalCg.readEntry("ShowHardware", true));

        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(d->notificationInterface, i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Select which kinds of information to show"));
        parent->addPage(d->configInterface, i18n("Auto Hide"), "window-suppressed");
    }

    QListWidget *visibleList = d->configInterface->availableListWidget();
    QListWidget *hiddenList = d->configInterface->selectedListWidget();

    visibleList->clear();
    hiddenList->clear();

    foreach (Task *task, Private::s_manager->tasks()) {
        if (!d->shownCategories.contains(task->category())) {
             continue;
        }

        if (!task->isHideable()) {
            continue;
        }

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setText(task->name());
        listItem->setIcon(task->icon());
        listItem->setData(Qt::UserRole, task->typeId());

        if (task->hidden() & Task::UserHidden) {
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
    globalCg.writeEntry("ShowJobs", d->ui.showJobs->isChecked());
    globalCg.writeEntry("ShowNotifications", d->ui.showNotifications->isChecked());

    disconnect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
               this, SLOT(addJob(SystemTray::Job*)));
    if (d->ui.showJobs->isChecked()) {
        createJobGroups();

        Private::s_manager->registerJobProtocol();
        connect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
    } else {
	Private::s_manager->unregisterJobProtocol();
    }

    disconnect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
               this, SLOT(addNotification(SystemTray::Notification*)));
    if (d->ui.showNotifications->isChecked()) {
        Private::s_manager->registerNotificationProtocol();
        connect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    } else {
	Private::s_manager->unregisterNotificationProtocol();
    }


    d->shownCategories.clear();

    globalCg.writeEntry("ShowApplicationStatus", d->ui.showApplicationStatus->isChecked());
    if (d->ui.showApplicationStatus->isChecked()) {
        d->shownCategories.insert(Task::ApplicationStatus);
    }

    globalCg.writeEntry("ShowCommunications", d->ui.showCommunications->isChecked());
    if (d->ui.showCommunications->isChecked()) {
        d->shownCategories.insert(Task::Communications);
    }

    globalCg.writeEntry("ShowSystemServices", d->ui.showSystemServices->isChecked());
    if (d->ui.showSystemServices->isChecked()) {
        d->shownCategories.insert(Task::SystemServices);
    }

    globalCg.writeEntry("ShowHardware", d->ui.showHardware->isChecked());
    if (d->ui.showHardware->isChecked()) {
        d->shownCategories.insert(Task::Hardware);
    }

    d->shownCategories.insert(Task::UnknownCategory);

    d->taskArea->syncTasks(manager()->tasks());
    emit configNeedsSaving();
}


void Applet::addNotification(Notification *notification)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "notification");
    extenderItem->setWidget(new NotificationWidget(notification, extenderItem));

    showPopup(autoHideTimeout);
}

void Applet::addJob(Job *job)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "job");
    extenderItem->setWidget(new JobWidget(job, extenderItem));

    showPopup(autoHideTimeout);

    extenderItem->setGroup(extender()->group("jobGroup"));
}

void Applet::initExtenderItem(Plasma::ExtenderItem *extenderItem)
{
    if (extenderItem->name() == "jobGroup") {
        d->jobSummaryWidget = new JobTotalsWidget(Private::s_manager->jobTotals(), extenderItem);
        extenderItem->setWidget(d->jobSummaryWidget);
        return;
    }

    if (extenderItem->name() == "completedJobsGroup") {
        Plasma::Label *label = new Plasma::Label(extenderItem);
        label->setText(QString("<div align='right'><a href='clear'>%1</a></div>").arg(i18n("clear all")));
        label->setPreferredSize(label->minimumSize());
        connect(label, SIGNAL(linkActivated(const QString &)), this, SLOT(clearAllCompletedJobs()));
        extenderItem->setWidget(label);
        return;
    }

    if (extenderItem->config().readEntry("type", "") == "notification") {
        extenderItem->setWidget(new NotificationWidget(0, extenderItem));
    } else if (extenderItem->config().readEntry("type", "") == "completedJob") {
        Plasma::Label *label = new Plasma::Label(extenderItem);

        QString destination = extenderItem->config().readEntry("destination", "");
        KUrl dest(destination);
        QString unformattedText;
        if (dest.isValid()) {
            //TODO: also check if it isn't a temp file
            label->setText(QString("%1: <a href='%2'>%3</a>")
                            .arg(i18n("Destination"))
                            .arg(dest.prettyUrl())
                            .arg(dest.prettyUrl()));
            connect(label, SIGNAL(linkActivated(const QString &)), this, SLOT(open(const QString &)));
            unformattedText = QString("%1: %2").arg(i18n("Destination")).arg(dest.prettyUrl());
        } else {
            label->setText(extenderItem->config().readEntry("text", ""));
            unformattedText = label->text();
        }

        QFontMetrics fm = label->nativeWidget()->fontMetrics();
        //sensible size hint:
        label->setPreferredSize(fm.boundingRect(QRect(0, 0, 300, 60),
                                  Qt::TextWordWrap, unformattedText).size());
        extenderItem->setWidget(label);
        extenderItem->showCloseButton();
    } else {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    }

}

void Applet::popupEvent(bool visibility)
{
    Q_UNUSED(visibility)

    if (!(Private::s_manager->jobs().isEmpty() &&
          Private::s_manager->notifications().isEmpty() &&
          extender()->group("completedJobsGroup")->items().isEmpty())) {
        if (!d->extenderTask) {
            d->extenderTask = new SystemTray::ExtenderTask(this, Private::s_manager, extender());
            d->extenderTask->setIcon("help-about");
        }

        Private::s_manager->addTask(d->extenderTask);
    }
}

void Applet::clearAllCompletedJobs()
{
    Plasma::ExtenderGroup *completedJobsGroup = extender()->group("completedJobsGroup");
    if (!completedJobsGroup) {
        return;
    }

    foreach (Plasma::ExtenderItem *item, completedJobsGroup->items()) {
        item->destroy();
    }
}

void Applet::finishJob(SystemTray::Job *job)
{
    Plasma::ExtenderItem *item = new Plasma::ExtenderItem(extender());
    item->setTitle(job->message());
    item->setIcon(job->applicationIconName());

    item->config().writeEntry("type", "completedJob");
    if (!job->error().isEmpty()) {
        item->config().writeEntry("text", job->error());
    } else {
        if (job->labels().count() > 1) {
            item->config().writeEntry("destination", job->labels().value(1).second);
            item->config().writeEntry("text", i18n("Destination: %1", job->labels().value(1).second));
        } else {
            item->config().writeEntry("text", QString("%1: %2").arg(job->labels().value(0).first)
                                                               .arg(job->labels().value(0).second));
        }
    }

    initExtenderItem(item);
    item->setGroup(extender()->group("completedJobsGroup"));
    showPopup(autoHideTimeout);
}

void Applet::open(const QString &url)
{
    QProcess::startDetached("kde-open", QStringList() << url);
}

void Applet::createJobGroups()
{
    if (!extender()->hasItem("jobGroup")) {
        Plasma::ExtenderGroup *extenderGroup = new Plasma::ExtenderGroup(extender());
        extenderGroup->setName("jobGroup");
        initExtenderItem(extenderGroup);
    }

    if (!extender()->hasItem("completedJobsGroup")) {
        Plasma::ExtenderGroup *completedJobsGroup = new Plasma::ExtenderGroup(extender());
        completedJobsGroup->setName("completedJobsGroup");
        completedJobsGroup->setTitle(i18n("Completed jobs"));
        initExtenderItem(completedJobsGroup);
        completedJobsGroup->expandGroup();
    }
}

}

#include "applet.moc"
