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
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>
#include <QtGui/QComboBox>
#include <QtCore/QProcess>


#include <KConfigDialog>

#include <Solid/Device>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/theme.h>
#include <plasma/dataenginemanager.h>
#include <plasma/dataengine.h>

#include "config.h"
#ifdef HAVE_LIBXSS      // Idle detection.
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <fixx11h.h>
#endif // HAVE_LIBXSS

#include "../core/manager.h"
#include "jobwidget.h"
#include "jobtotalswidget.h"
#include "notificationwidget.h"
#include "taskarea.h"

#include "ui_protocols.h"
#include "ui_autohide.h"
#include "ui_plasmoidtasks.h"

namespace SystemTray
{


K_EXPORT_PLASMA_APPLET(systemtray, Applet)


class Applet::Private
{
public:
    Private(Applet *q)
        : q(q),
          taskArea(0),
          notificationInterface(0),
          autoHideInterface(0),
          background(0),
          jobSummaryWidget(0),
          timerId(0)
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
    QPointer<QWidget> notificationInterface;
    QPointer<QWidget> autoHideInterface;
    QPointer<QWidget> plasmoidTasksInterface;
    QList<Job*> jobs;
    QSet<Task::Category> shownCategories;
    QDateTime lastActivity;

    Plasma::FrameSvg *background;
    Plasma::Svg *icons;
    JobTotalsWidget *jobSummaryWidget;
    static SystemTray::Manager *s_manager;
    static int s_managerUsage;
    int autoHideTimeout;
    int timerId;

    Ui::ProtocolsConfig notificationUi;
    Ui::AutoHideConfig autoHideUi;
    Ui::PlasmoidTasksConfig plasmoidTasksUi;
};

Manager *Applet::Private::s_manager = 0;
int Applet::Private::s_managerUsage = 0;
static const int idleCheckInterval = 60 * 1000;
static const int completedJobExpireDelay = 5 * 60 * 1000;

Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      d(new Private(this))
{
    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/systemtray");
    d->background->setCacheAllRenderedFrames(true);
    d->taskArea = new TaskArea(this);

    d->icons = new Plasma::Svg(this);
    d->icons->setImagePath("widgets/configuration-icons");

    setPopupIcon(QIcon());
    setPassivePopup(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
    QAction *addDefaultApplets = new QAction(i18n("add default applets"), this);
    connect(addDefaultApplets, SIGNAL(triggered()), this, SLOT(addDefaultApplets()));
    addAction("add default applets", addDefaultApplets);
}

Applet::~Applet()
{
    // stop listening to the manager
    disconnect(Private::s_manager, 0, this, 0);

    // remove the taskArea so we can delete the widgets without it going nuts on us
    delete d->taskArea;

    foreach (Task *task, Private::s_manager->tasks()) {
        // we don't care about the task updates anymore
        disconnect(task, 0, this, 0);

        // delete our widget (if any); some widgets (such as the extender info one)
        // may rely on the applet being around, so we need to delete them here and now
        // while we're still kicking
        delete task->widget(this, false);
    }

    delete d;
}

void Applet::init()
{
    KConfigGroup cg = config();
    QStringList hiddenTypes = cg.readEntry("hidden", QStringList());
    QStringList alwaysShownTypes = cg.readEntry("alwaysShown", QStringList());

    d->setTaskAreaGeometry();
    connect(Private::s_manager, SIGNAL(taskAdded(SystemTray::Task*)),
            d->taskArea, SLOT(addTask(SystemTray::Task*)));
    //TODO: we re-add the task when it changed: slightly silly!
    connect(Private::s_manager, SIGNAL(taskChanged(SystemTray::Task*)),
            d->taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(Private::s_manager, SIGNAL(taskRemoved(SystemTray::Task*)),
            d->taskArea, SLOT(removeTask(SystemTray::Task*)));

    d->taskArea->setHiddenTypes(hiddenTypes);
    d->taskArea->setAlwaysShownTypes(alwaysShownTypes);
    connect(d->taskArea, SIGNAL(sizeHintChanged(Qt::SizeHint)),
            this, SLOT(propogateSizeHintChange(Qt::SizeHint)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(checkSizes()));
    checkSizes();

    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));
    extender()->setWindowFlags(Qt::X11BypassWindowManagerHint);

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

    if (config().readEntry("AutoHidePopup", true)) {
        d->autoHideTimeout = 6000;
    } else {
        d->autoHideTimeout = 0;
    }

    d->shownCategories.insert(Task::UnknownCategory);

    //destroy any item in the systray, that doesn't belong to the completedJobsGroup, since running
    //jobs and notifications can't really survive reboots anyways
    foreach (Plasma::ExtenderItem *item, extender()->attachedItems()) {
        if (!item->isGroup() && (item->group() != extender()->group("completedJobsGroup"))) {
            item->destroy();
        }
    }

    bool createExtenderTask = false;

    if (globalCg.readEntry("ShowJobs", true)) {
        createExtenderTask = true;
        createJobGroups();

        Private::s_manager->registerJobProtocol();
        connect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
        connect(Private::s_manager, SIGNAL(jobRemoved(SystemTray::Job*)),
                this, SLOT(finishJob(SystemTray::Job*)));
    }

    if (globalCg.readEntry("ShowNotifications", true)) {
        createExtenderTask = true;
        Private::s_manager->registerNotificationProtocol();
        connect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    }

    initExtenderTask(createExtenderTask);
    Private::s_manager->loadApplets(config(), this);
    d->taskArea->syncTasks(Private::s_manager->tasks());
}

void Applet::initExtenderTask(bool create)
{
    if (create) {
        extender(); // make sure it exists
        d->taskArea->addTask(Private::s_manager->extenderTask());
    } else if (Private::s_manager->extenderTask(false)) {
        d->taskArea->removeTask(Private::s_manager->extenderTask());
        QGraphicsWidget *widget = Private::s_manager->extenderTask(false)->widget(this, false);
        widget->deleteLater();
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

    d->s_manager->forwardConstraintsEvent(constraints);
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
    Plasma::FormFactor f = formFactor();
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    d->background->setElementPrefix(QString());
    d->background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    d->background->getMargins(leftMargin, topMargin, rightMargin, bottomMargin);

    QSizeF minSize = d->taskArea->effectiveSizeHint(Qt::MinimumSize);
    if (f == Plasma::Horizontal && minSize.height() >= size().height() - topMargin - bottomMargin) {
        d->background->setElementPrefix(QString());
        d->background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
        d->background->setElementPrefix("lastelements");
        d->background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
        setContentsMargins(leftMargin, 0, rightMargin, 0);
    } else if (f == Plasma::Vertical && minSize.width() >= size().width() - leftMargin - rightMargin) {
        d->background->setElementPrefix(QString());
        d->background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder);
        d->background->setElementPrefix("lastelements");
        d->background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder);
        setContentsMargins(0, topMargin, 0, bottomMargin);
    } else {
        d->background->setElementPrefix(QString());
        d->background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        d->background->setElementPrefix("lastelements");
        d->background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);
    }

    QSizeF preferredSize = d->taskArea->effectiveSizeHint(Qt::PreferredSize);
    preferredSize.setWidth(preferredSize.width() + leftMargin + rightMargin);
    preferredSize.setHeight(preferredSize.height() + topMargin + bottomMargin);
    setPreferredSize(preferredSize);

    QSizeF actualSize = size();

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

        if (formFactor() == Plasma::Vertical && d->background->hasElement("horizontal-separator")) {
            QSize s = d->background->elementRect("horizontal-separator").size().toSize();
            d->background->paint(painter, QRect(lastRect.topLeft() - QPoint(0, s.height() / 2),
                                                QSize(lastRect.width(), s.height())), "horizontal-separator");
        } else if (QApplication::layoutDirection() == Qt::RightToLeft && d->background->hasElement("vertical-separator")) {
            QSize s = d->background->elementRect("vertical-separator").size().toSize();
            d->background->paint(painter, QRect(lastRect.topRight() - QPoint(s.width() / 2, 0),
                                                QSize(s.width(), lastRect.height())), "vertical-separator");
        } else if (d->background->hasElement("vertical-separator")) {
            QSize s = d->background->elementRect("vertical-separator").size().toSize();
            d->background->paint(painter, QRect(lastRect.topLeft() - QPoint(s.width() / 2, 0),
                                                QSize(s.width(), lastRect.height())), "vertical-separator");
        }
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
    if (!d->autoHideInterface) {
        KConfigGroup globalCg = globalConfig();
        d->notificationInterface = new QWidget();
        d->autoHideInterface = new QWidget();
        d->plasmoidTasksInterface = new QWidget();

        d->notificationUi.setupUi(d->notificationInterface);

        d->notificationUi.showJobs->setChecked(globalCg.readEntry("ShowJobs", true));
        d->notificationUi.showNotifications->setChecked(globalCg.readEntry("ShowNotifications", true));

        d->notificationUi.showApplicationStatus->setChecked(globalCg.readEntry("ShowApplicationStatus", true));
        d->notificationUi.showCommunications->setChecked(globalCg.readEntry("ShowCommunications", true));
        d->notificationUi.showSystemServices->setChecked(globalCg.readEntry("ShowSystemServices", true));
        d->notificationUi.showHardware->setChecked(globalCg.readEntry("ShowHardware", true));

        d->autoHideUi.setupUi(d->autoHideInterface);
        d->autoHideUi.autoHide->setChecked(config().readEntry("AutoHidePopup", true));

        d->plasmoidTasksUi.setupUi(d->plasmoidTasksInterface);


        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(d->notificationInterface, i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));
        parent->addPage(d->autoHideInterface, i18n("Auto Hide"), "window-suppressed");
        parent->addPage(d->plasmoidTasksInterface, i18n("Plasma widgets"), "plasma");
    }

    d->autoHideUi.icons->clear();
    d->plasmoidTasksUi.applets->clear();

    QMultiMap<QString, const Task *> sortedTasks;
    foreach (const Task *task, Private::s_manager->tasks()) {
        if (!d->shownCategories.contains(task->category())) {
             continue;
        }

        if (!task->isHideable()) {
            continue;
        }

        sortedTasks.insert(task->name(), task);
    }

    foreach (const Task *task, sortedTasks) {
        QTreeWidgetItem *listItem = new QTreeWidgetItem(d->autoHideUi.icons);
        QComboBox *itemCombo = new QComboBox(d->autoHideUi.icons);
        listItem->setText(0, task->name());
        listItem->setIcon(0, task->icon());
        listItem->setFlags(Qt::ItemIsEnabled);
        listItem->setData(0, Qt::UserRole, task->typeId());

        itemCombo->addItem(i18nc("Item will be automatically shown or hidden from the systray", "Auto"));
        itemCombo->addItem(i18nc("Item is never visible in the systray", "Hidden"));
        itemCombo->addItem(i18nc("Item is always visible in the systray", "Always visible"));

        if (task->hidden() & Task::UserHidden) {
            itemCombo->setCurrentIndex(1);
        } else if (d->taskArea->alwaysShownTypes().contains(task->typeId())) {
            itemCombo->setCurrentIndex(2);
        } else {
            itemCombo->setCurrentIndex(0);
        }
        d->autoHideUi.icons->setItemWidget(listItem, 1, itemCombo);

        d->autoHideUi.icons->addTopLevelItem(listItem);
    }

    QStringList ownApplets = Private::s_manager->applets(this);
    foreach (const KPluginInfo &info, Plasma::Applet::listAppletInfo()) {
        KService::Ptr service = info.service();
        if (service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
            QListWidgetItem *listItem = new QListWidgetItem();
            listItem->setText(service->name());
            listItem->setIcon(KIcon(service->icon()));
            listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            listItem->setData(Qt::UserRole, info.pluginName());
            listItem->setCheckState(ownApplets.contains(info.pluginName()) ? Qt::Checked : Qt::Unchecked);
            d->plasmoidTasksUi.applets->addItem(listItem);
        }
    }

}

void Applet::configAccepted()
{
    QStringList hiddenTypes;
    QStringList alwaysShownTypes;
    QTreeWidget *hiddenList = d->autoHideUi.icons;
    for (int i = 0; i < hiddenList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = hiddenList->topLevelItem(i);
        QComboBox *itemCombo = static_cast<QComboBox *>(hiddenList->itemWidget(item, 1));
        //kDebug() << (item->checkState() == Qt::Checked) << item->data(Qt::UserRole).toString();
        //Always hidden
        if (itemCombo->currentIndex() == 1) {
            hiddenTypes << item->data(0, Qt::UserRole).toString();
        //Always visible
        } else if (itemCombo->currentIndex() == 2) {
            alwaysShownTypes << item->data(0, Qt::UserRole).toString();
        }
    }

    d->taskArea->setHiddenTypes(hiddenTypes);
    d->taskArea->setAlwaysShownTypes(alwaysShownTypes);
    d->taskArea->syncTasks(Private::s_manager->tasks());

    KConfigGroup cg = config();
    cg.writeEntry("hidden", hiddenTypes);
    cg.writeEntry("alwaysShown", alwaysShownTypes);

    cg.writeEntry("AutoHidePopup", d->autoHideUi.autoHide->isChecked());
    if (d->autoHideUi.autoHide->isChecked()) {
        d->autoHideTimeout = 6000;
    } else {
        d->autoHideTimeout = 0;
    }

    KConfigGroup globalCg = globalConfig();
    globalCg.writeEntry("ShowJobs", d->notificationUi.showJobs->isChecked());
    globalCg.writeEntry("ShowNotifications", d->notificationUi.showNotifications->isChecked());
    bool createExtenderTask = false;

    disconnect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
               this, SLOT(addJob(SystemTray::Job*)));
    if (d->notificationUi.showJobs->isChecked()) {
        createJobGroups();
        createExtenderTask = true;

        Private::s_manager->registerJobProtocol();
        connect(Private::s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
    } else {
        Private::s_manager->unregisterJobProtocol();
    }

    disconnect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
               this, SLOT(addNotification(SystemTray::Notification*)));
    if (d->notificationUi.showNotifications->isChecked()) {
        createExtenderTask = true;
        Private::s_manager->registerNotificationProtocol();
        connect(Private::s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    } else {
        Private::s_manager->unregisterNotificationProtocol();
    }


    initExtenderTask(createExtenderTask);
    d->shownCategories.clear();

    globalCg.writeEntry("ShowApplicationStatus", d->notificationUi.showApplicationStatus->isChecked());
    if (d->notificationUi.showApplicationStatus->isChecked()) {
        d->shownCategories.insert(Task::ApplicationStatus);
    }

    globalCg.writeEntry("ShowCommunications", d->notificationUi.showCommunications->isChecked());
    if (d->notificationUi.showCommunications->isChecked()) {
        d->shownCategories.insert(Task::Communications);
    }

    globalCg.writeEntry("ShowSystemServices", d->notificationUi.showSystemServices->isChecked());
    if (d->notificationUi.showSystemServices->isChecked()) {
        d->shownCategories.insert(Task::SystemServices);
    }

    globalCg.writeEntry("ShowHardware", d->notificationUi.showHardware->isChecked());
    if (d->notificationUi.showHardware->isChecked()) {
        d->shownCategories.insert(Task::Hardware);
    }

    d->shownCategories.insert(Task::UnknownCategory);

    d->taskArea->syncTasks(manager()->tasks());

    QStringList applets = Private::s_manager->applets(this);
    for (int i = 0; i < d->plasmoidTasksUi.applets->count(); ++i) {
        QListWidgetItem * item = d->plasmoidTasksUi.applets->item(i);
        QString appletName = item->data(Qt::UserRole).toString();

        if (item->checkState() != Qt::Unchecked && !applets.contains(appletName)) {
            Private::s_manager->addApplet(appletName, this);
        }

        if (item->checkState() == Qt::Checked) {
            applets.removeAll(appletName);
        }
    }

    foreach (QString appletName, applets) {
        Private::s_manager->removeApplet(appletName, this);
    }

    emit configNeedsSaving();
}

void Applet::addDefaultApplets()
{
    QStringList applets = Private::s_manager->applets(this);
    if (!applets.contains("notifier")) {
        Private::s_manager->addApplet("notifier", this);
    }
    if (!applets.contains("battery")) {
        Plasma::DataEngineManager *engines = Plasma::DataEngineManager::self();
        Plasma::DataEngine *power = engines->loadEngine("powermanagement");
        if (power) {
            const QStringList &batteries = power->query("Battery")["sources"].toStringList();
            if (!batteries.isEmpty()) {
                Private::s_manager->addApplet("battery", this);
            }
        }
        engines->unloadEngine("powermanagement");
    }
}

void Applet::addNotification(Notification *notification)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "notification");
    extenderItem->setWidget(new NotificationWidget(notification, extenderItem));

    showPopup(d->autoHideTimeout);
}

void Applet::addJob(Job *job)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "job");
    extenderItem->setWidget(new JobWidget(job, extenderItem));

    showPopup(d->autoHideTimeout);

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
        QGraphicsWidget *widget = new QGraphicsWidget(this);
        widget->setMaximumHeight(0);
        extenderItem->setWidget(widget);

        QAction *clearAction = new QAction(this);
        clearAction->setIcon(KIcon(d->icons->pixmap("close")));
        extenderItem->addAction("space", new QAction(this));
        extenderItem->addAction("clear", clearAction);
        connect(clearAction, SIGNAL(triggered()), this, SLOT(clearAllCompletedJobs()));
        return;
    }

    if (extenderItem->config().readEntry("type", "") == "notification") {
        extenderItem->setWidget(new NotificationWidget(0, extenderItem));
    } else if (extenderItem->config().readEntry("type", "") == "completedJob") {
        Plasma::Label *label = new Plasma::Label(extenderItem);
        label->nativeWidget()->setLineWidth(300);
        label->setMinimumWidth(300);
        label->setText(extenderItem->config().readEntry("text", ""));
        label->setPreferredSize(label->minimumSize());

        extenderItem->setWidget(label);
        extenderItem->showCloseButton();
    } else {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    }

}

void Applet::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->timerId) {
        Plasma::Applet::timerEvent(event);
        return;
    }

    int totalIdle;
#ifdef HAVE_LIBXSS      // Idle detection.
    XScreenSaverInfo*  _mit_info;
    _mit_info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo( QX11Info::display(), QX11Info::appRootWindow(), _mit_info );
    totalIdle =  _mit_info->idle;
    XFree( _mit_info );
#else
    totalIdle = 0;
#endif // HAVE_LIBXSS

    if (totalIdle < idleCheckInterval) {
        Plasma::ExtenderGroup *group = extender()->group("completedJobsGroup");
        if (group) {
            foreach (Plasma::ExtenderItem *item, group->items()) {
                if (!item->autoExpireDelay()) {
                    item->setAutoExpireDelay(completedJobExpireDelay);
                }
            }
        }

        killTimer(d->timerId);
        d->timerId = 0;
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
    item->setTitle(i18n("%1 [Finished]", job->message()));
    item->setIcon(job->applicationIconName());

    item->config().writeEntry("type", "completedJob");
    if (job->error().isEmpty()) {
        item->config().writeEntry("text", job->completedMessage());
    } else {
        item->config().writeEntry("text", job->error());
    }

    initExtenderItem(item);
    item->setGroup(extender()->group("completedJobsGroup"));
    showPopup(d->autoHideTimeout);
    if (!d->timerId) {
        d->timerId = startTimer(idleCheckInterval);
    }
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
        completedJobsGroup->setTitle(i18n("Recently Completed Jobs"));
        initExtenderItem(completedJobsGroup);
        completedJobsGroup->expandGroup();
    }
}

}

#include "applet.moc"
