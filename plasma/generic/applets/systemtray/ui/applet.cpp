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
#include <QtCore/QProcess>


#include <KConfigDialog>
#include <KComboBox>

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

namespace SystemTray
{


K_EXPORT_PLASMA_APPLET(systemtray, Applet)


Manager *Applet::s_manager = 0;
int Applet::s_managerUsage = 0;
static const int idleCheckInterval = 60 * 1000;
static const int completedJobExpireDelay = 5 * 60 * 1000;

Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      m_taskArea(0),
      m_background(0),
      m_jobSummaryWidget(0),
      m_timerId(0)
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
    }

    ++s_managerUsage;

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/systemtray");
    m_background->setCacheAllRenderedFrames(true);
    m_taskArea = new TaskArea(this);

    m_icons = new Plasma::Svg(this);
    m_icons->setImagePath("widgets/configuration-icons");

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
    disconnect(s_manager, 0, this, 0);

    // remove the taskArea so we can delete the widgets without it going nuts on us
    delete m_taskArea;

    foreach (Task *task, s_manager->tasks()) {
        // we don't care about the task updates anymore
        disconnect(task, 0, this, 0);

        // delete our widget (if any); some widgets (such as the extender info one)
        // may rely on the applet being around, so we need to delete them here and now
        // while we're still kicking
        delete task->widget(this, false);
    }

    clearAllCompletedJobs();

    --s_managerUsage;
    if (s_managerUsage < 1) {
        delete s_manager;
        s_manager = 0;
        s_managerUsage = 0;
    }
}

void Applet::init()
{
    KConfigGroup cg = config();
    QStringList hiddenTypes = cg.readEntry("hidden", QStringList());
    QStringList alwaysShownTypes = cg.readEntry("alwaysShown", QStringList());

    setTaskAreaGeometry();
    connect(s_manager, SIGNAL(taskAdded(SystemTray::Task*)),
            m_taskArea, SLOT(addTask(SystemTray::Task*)));
    //TODO: we re-add the task when it changed: slightly silly!
    connect(s_manager, SIGNAL(taskChanged(SystemTray::Task*)),
            m_taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(s_manager, SIGNAL(taskRemoved(SystemTray::Task*)),
            m_taskArea, SLOT(removeTask(SystemTray::Task*)));

    m_taskArea->setHiddenTypes(hiddenTypes);
    m_taskArea->setAlwaysShownTypes(alwaysShownTypes);
    connect(m_taskArea, SIGNAL(sizeHintChanged(Qt::SizeHint)),
            this, SLOT(propogateSizeHintChange(Qt::SizeHint)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(checkSizes()));
    checkSizes();

    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));
    extender()->setWindowFlags(Qt::X11BypassWindowManagerHint);

    KConfigGroup globalCg = globalConfig();

    if (globalCg.readEntry("ShowApplicationStatus", true)) {
        m_shownCategories.insert(Task::ApplicationStatus);
    }
    if (globalCg.readEntry("ShowCommunications", true)) {
        m_shownCategories.insert(Task::Communications);
    }
    if (globalCg.readEntry("ShowSystemServices", true)) {
        m_shownCategories.insert(Task::SystemServices);
    }
    if (globalCg.readEntry("ShowHardware", true)) {
        m_shownCategories.insert(Task::Hardware);
    }

    if (config().readEntry("AutoHidePopup", true)) {
        m_autoHideTimeout = 6000;
    } else {
        m_autoHideTimeout = 0;
    }

    m_shownCategories.insert(Task::UnknownCategory);

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

        s_manager->registerJobProtocol();
        connect(s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
        connect(s_manager, SIGNAL(jobRemoved(SystemTray::Job*)),
                this, SLOT(finishJob(SystemTray::Job*)));
    }

    if (globalCg.readEntry("ShowNotifications", true)) {
        createExtenderTask = true;
        s_manager->registerNotificationProtocol();
        connect(s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    }

    initExtenderTask(createExtenderTask);
    s_manager->loadApplets(config(), this);
    m_taskArea->syncTasks(s_manager->tasks());
}

void Applet::initExtenderTask(bool create)
{
    if (create) {
        extender(); // make sure it exists
        m_taskArea->addTask(s_manager->extenderTask());
    } else if (s_manager->extenderTask(false)) {
        m_taskArea->removeTask(s_manager->extenderTask());
        QGraphicsWidget *widget = s_manager->extenderTask(false)->widget(this, false);
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
        m_taskArea->setSizePolicy(policy);
        m_taskArea->setOrientation(vertical ? Qt::Vertical : Qt::Horizontal);
    }

    if (constraints & Plasma::SizeConstraint) {
        checkSizes();
    }

    s_manager->forwardConstraintsEvent(constraints);
}

SystemTray::Manager *Applet::manager() const
{
    return s_manager;
}

QSet<Task::Category> Applet::shownCategories() const
{
    return m_shownCategories;
}

void Applet::setGeometry(const QRectF &rect)
{
    Plasma::Applet::setGeometry(rect);

    if (m_taskArea) {
        setTaskAreaGeometry();
    }
}

void Applet::checkSizes()
{
    Plasma::FormFactor f = formFactor();
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    m_background->setElementPrefix(QString());
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_background->getMargins(leftMargin, topMargin, rightMargin, bottomMargin);

    QSizeF minSize = m_taskArea->effectiveSizeHint(Qt::MinimumSize);
    if (f == Plasma::Horizontal && minSize.height() >= size().height() - topMargin - bottomMargin) {
        m_background->setElementPrefix(QString());
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
        m_background->setElementPrefix("lastelements");
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
        setContentsMargins(leftMargin, 0, rightMargin, 0);
    } else if (f == Plasma::Vertical && minSize.width() >= size().width() - leftMargin - rightMargin) {
        m_background->setElementPrefix(QString());
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder);
        m_background->setElementPrefix("lastelements");
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder);
        setContentsMargins(0, topMargin, 0, bottomMargin);
    } else {
        m_background->setElementPrefix(QString());
        m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        m_background->setElementPrefix("lastelements");
        m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);
    }

    QSizeF preferredSize = m_taskArea->effectiveSizeHint(Qt::PreferredSize);
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

        preferredSize = m_taskArea->effectiveSizeHint(Qt::PreferredSize, actualSize);
        preferredSize.setWidth(qMax(actualSize.width(), preferredSize.width()));
        preferredSize.setHeight(qMax(actualSize.height(), preferredSize.height()));

        resize(preferredSize);
    }
}


void Applet::setTaskAreaGeometry()
{
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    QRectF taskAreaRect(rect());
    taskAreaRect.moveLeft(leftMargin);
    taskAreaRect.moveTop(topMargin);
    taskAreaRect.setWidth(taskAreaRect.width() - leftMargin - rightMargin);
    taskAreaRect.setHeight(taskAreaRect.height() - topMargin - bottomMargin);

    m_taskArea->setGeometry(taskAreaRect);
}

void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

    QRect normalRect = rect().toRect();
    QRect lastRect(normalRect);
    m_background->setElementPrefix("lastelements");

    if (formFactor() == Plasma::Vertical) {
        const int rightEasement = m_taskArea->rightEasement() + m_background->marginSize(Plasma::BottomMargin);
        normalRect.setY(m_taskArea->leftEasement());
        normalRect.setBottom(normalRect.bottom() - rightEasement);

        lastRect.setY(normalRect.bottom() + 1);
        lastRect.setHeight(rightEasement);
    } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
        const int rightEasement = m_taskArea->rightEasement() + m_background->marginSize(Plasma::LeftMargin);
        normalRect.setWidth(normalRect.width() - m_taskArea->leftEasement());
        normalRect.setLeft(rightEasement);

        lastRect.setX(0);
        lastRect.setWidth(rightEasement);
    } else {
        const int rightEasement = m_taskArea->rightEasement() + m_background->marginSize(Plasma::RightMargin);
        normalRect.setX(m_taskArea->leftEasement());
        normalRect.setWidth(normalRect.width() - rightEasement);

        lastRect.setX(normalRect.right() + 1);
        lastRect.setWidth(rightEasement);
    }

    QRect r = normalRect.united(lastRect);

    painter->save();

    m_background->setElementPrefix(QString());
    m_background->resizeFrame(r.size());
    if (m_taskArea->rightEasement() > 0) {
        painter->setClipRect(normalRect);
    }
    m_background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));

    if (m_taskArea->rightEasement() > 0) {
        m_background->setElementPrefix("lastelements");
        m_background->resizeFrame(r.size());
        painter->setClipRect(lastRect);
        m_background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));

        if (formFactor() == Plasma::Vertical && m_background->hasElement("horizontal-separator")) {
            QSize s = m_background->elementRect("horizontal-separator").size().toSize();
            m_background->paint(painter, QRect(lastRect.topLeft() - QPoint(0, s.height() / 2),
                                                QSize(lastRect.width(), s.height())), "horizontal-separator");
        } else if (QApplication::layoutDirection() == Qt::RightToLeft && m_background->hasElement("vertical-separator")) {
            QSize s = m_background->elementRect("vertical-separator").size().toSize();
            m_background->paint(painter, QRect(lastRect.topRight() - QPoint(s.width() / 2, 0),
                                                QSize(s.width(), lastRect.height())), "vertical-separator");
        } else if (m_background->hasElement("vertical-separator")) {
            QSize s = m_background->elementRect("vertical-separator").size().toSize();
            m_background->paint(painter, QRect(lastRect.topLeft() - QPoint(s.width() / 2, 0),
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
    if (!m_autoHideInterface) {
        KConfigGroup globalCg = globalConfig();
        m_notificationInterface = new QWidget();
        m_autoHideInterface = new QWidget();
        m_plasmoidTasksInterface = new QWidget();

        m_notificationUi.setupUi(m_notificationInterface.data());

        m_notificationUi.showJobs->setChecked(globalCg.readEntry("ShowJobs", true));
        m_notificationUi.showNotifications->setChecked(globalCg.readEntry("ShowNotifications", true));

        m_notificationUi.showApplicationStatus->setChecked(globalCg.readEntry("ShowApplicationStatus", true));
        m_notificationUi.showCommunications->setChecked(globalCg.readEntry("ShowCommunications", true));
        m_notificationUi.showSystemServices->setChecked(globalCg.readEntry("ShowSystemServices", true));
        m_notificationUi.showHardware->setChecked(globalCg.readEntry("ShowHardware", true));

        m_autoHideUi.setupUi(m_autoHideInterface.data());
        m_autoHideUi.autoHide->setChecked(config().readEntry("AutoHidePopup", true));

        m_plasmoidTasksUi.setupUi(m_plasmoidTasksInterface.data());


        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(m_notificationInterface.data(), i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));
        parent->addPage(m_autoHideInterface.data(), i18n("Auto Hide"), "window-suppressed");
        parent->addPage(m_plasmoidTasksInterface.data(), i18n("Plasma widgets"), "plasma");
    }

    m_autoHideUi.icons->clear();
    m_plasmoidTasksUi.applets->clear();

    QMultiMap<QString, const Task *> sortedTasks;
    foreach (const Task *task, s_manager->tasks()) {
        if (!m_shownCategories.contains(task->category())) {
             continue;
        }

        if (!task->isHideable()) {
            continue;
        }

        sortedTasks.insert(task->name(), task);
    }

    foreach (const Task *task, sortedTasks) {
        QTreeWidgetItem *listItem = new QTreeWidgetItem(m_autoHideUi.icons);
        KComboBox *itemCombo = new KComboBox(m_autoHideUi.icons);
        listItem->setText(0, task->name());
        listItem->setIcon(0, task->icon());
        listItem->setFlags(Qt::ItemIsEnabled);
        listItem->setData(0, Qt::UserRole, task->typeId());

        itemCombo->addItem(i18nc("Item will be automatically shown or hidden from the systray", "Auto"));
        itemCombo->addItem(i18nc("Item is never visible in the systray", "Hidden"));
        itemCombo->addItem(i18nc("Item is always visible in the systray", "Always visible"));

        if (task->hidden() & Task::UserHidden) {
            itemCombo->setCurrentIndex(1);
        } else if (m_taskArea->alwaysShownTypes().contains(task->typeId())) {
            itemCombo->setCurrentIndex(2);
        } else {
            itemCombo->setCurrentIndex(0);
        }
        m_autoHideUi.icons->setItemWidget(listItem, 1, itemCombo);

        m_autoHideUi.icons->addTopLevelItem(listItem);
    }

    QStringList ownApplets = s_manager->applets(this);
    foreach (const KPluginInfo &info, Plasma::Applet::listAppletInfo()) {
        KService::Ptr service = info.service();
        if (service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
            QListWidgetItem *listItem = new QListWidgetItem();
            listItem->setText(service->name());
            listItem->setIcon(KIcon(service->icon()));
            listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            listItem->setData(Qt::UserRole, info.pluginName());
            listItem->setCheckState(ownApplets.contains(info.pluginName()) ? Qt::Checked : Qt::Unchecked);
            m_plasmoidTasksUi.applets->addItem(listItem);
        }
    }

}

void Applet::configAccepted()
{
    QStringList hiddenTypes;
    QStringList alwaysShownTypes;
    QTreeWidget *hiddenList = m_autoHideUi.icons;
    for (int i = 0; i < hiddenList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = hiddenList->topLevelItem(i);
        KComboBox *itemCombo = static_cast<KComboBox *>(hiddenList->itemWidget(item, 1));
        //kDebug() << (item->checkState() == Qt::Checked) << item->data(Qt::UserRole).toString();
        //Always hidden
        if (itemCombo->currentIndex() == 1) {
            hiddenTypes << item->data(0, Qt::UserRole).toString();
        //Always visible
        } else if (itemCombo->currentIndex() == 2) {
            alwaysShownTypes << item->data(0, Qt::UserRole).toString();
        }
    }

    m_taskArea->setHiddenTypes(hiddenTypes);
    m_taskArea->setAlwaysShownTypes(alwaysShownTypes);
    m_taskArea->syncTasks(s_manager->tasks());

    KConfigGroup cg = config();
    cg.writeEntry("hidden", hiddenTypes);
    cg.writeEntry("alwaysShown", alwaysShownTypes);

    cg.writeEntry("AutoHidePopup", m_autoHideUi.autoHide->isChecked());
    if (m_autoHideUi.autoHide->isChecked()) {
        m_autoHideTimeout = 6000;
    } else {
        m_autoHideTimeout = 0;
    }

    KConfigGroup globalCg = globalConfig();
    globalCg.writeEntry("ShowJobs", m_notificationUi.showJobs->isChecked());
    globalCg.writeEntry("ShowNotifications", m_notificationUi.showNotifications->isChecked());
    bool createExtenderTask = false;

    disconnect(s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
               this, SLOT(addJob(SystemTray::Job*)));
    if (m_notificationUi.showJobs->isChecked()) {
        createJobGroups();
        createExtenderTask = true;

        s_manager->registerJobProtocol();
        connect(s_manager, SIGNAL(jobAdded(SystemTray::Job*)),
                this, SLOT(addJob(SystemTray::Job*)));
    } else {
        s_manager->unregisterJobProtocol();
    }

    disconnect(s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
               this, SLOT(addNotification(SystemTray::Notification*)));
    if (m_notificationUi.showNotifications->isChecked()) {
        createExtenderTask = true;
        s_manager->registerNotificationProtocol();
        connect(s_manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
                this, SLOT(addNotification(SystemTray::Notification*)));
    } else {
        s_manager->unregisterNotificationProtocol();
    }


    initExtenderTask(createExtenderTask);
    m_shownCategories.clear();

    globalCg.writeEntry("ShowApplicationStatus", m_notificationUi.showApplicationStatus->isChecked());
    if (m_notificationUi.showApplicationStatus->isChecked()) {
        m_shownCategories.insert(Task::ApplicationStatus);
    }

    globalCg.writeEntry("ShowCommunications", m_notificationUi.showCommunications->isChecked());
    if (m_notificationUi.showCommunications->isChecked()) {
        m_shownCategories.insert(Task::Communications);
    }

    globalCg.writeEntry("ShowSystemServices", m_notificationUi.showSystemServices->isChecked());
    if (m_notificationUi.showSystemServices->isChecked()) {
        m_shownCategories.insert(Task::SystemServices);
    }

    globalCg.writeEntry("ShowHardware", m_notificationUi.showHardware->isChecked());
    if (m_notificationUi.showHardware->isChecked()) {
        m_shownCategories.insert(Task::Hardware);
    }

    m_shownCategories.insert(Task::UnknownCategory);

    m_taskArea->syncTasks(manager()->tasks());

    QStringList applets = s_manager->applets(this);
    for (int i = 0; i < m_plasmoidTasksUi.applets->count(); ++i) {
        QListWidgetItem * item = m_plasmoidTasksUi.applets->item(i);
        QString appletName = item->data(Qt::UserRole).toString();

        if (item->checkState() != Qt::Unchecked && !applets.contains(appletName)) {
            s_manager->addApplet(appletName, this);
        }

        if (item->checkState() == Qt::Checked) {
            applets.removeAll(appletName);
        }
    }

    foreach (const QString &appletName, applets) {
        s_manager->removeApplet(appletName, this);
    }

    emit configNeedsSaving();
}

void Applet::addDefaultApplets()
{
    QStringList applets = s_manager->applets(this);
    if (!applets.contains("notifier")) {
        s_manager->addApplet("notifier", this);
    }
    if (!applets.contains("battery")) {
        Plasma::DataEngineManager *engines = Plasma::DataEngineManager::self();
        Plasma::DataEngine *power = engines->loadEngine("powermanagement");
        if (power) {
            const QStringList &batteries = power->query("Battery")["sources"].toStringList();
            if (!batteries.isEmpty()) {
                s_manager->addApplet("battery", this);
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

    showPopup(m_autoHideTimeout);
}

void Applet::addJob(Job *job)
{
    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender());
    extenderItem->config().writeEntry("type", "job");
    extenderItem->setWidget(new JobWidget(job, extenderItem));

    showPopup(m_autoHideTimeout);

    extenderItem->setGroup(extender()->group("jobGroup"));
}

void Applet::initExtenderItem(Plasma::ExtenderItem *extenderItem)
{
    if (extenderItem->name() == "jobGroup") {
        m_jobSummaryWidget = new JobTotalsWidget(s_manager->jobTotals(), extenderItem);
        extenderItem->setWidget(m_jobSummaryWidget);
        return;
    }

    if (extenderItem->name() == "completedJobsGroup") {
        QGraphicsWidget *widget = new QGraphicsWidget(this);
        widget->setMaximumHeight(0);
        extenderItem->setWidget(widget);

        QAction *clearAction = new QAction(this);
        clearAction->setIcon(KIcon(m_icons->pixmap("close")));
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
        connect(label, SIGNAL(linkActivated(const QString &)),
                this, SLOT(open(const QString &)));

        extenderItem->setWidget(label);
        extenderItem->showCloseButton();
    } else {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    }

}

void Applet::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timerId) {
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

        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void Applet::popupEvent(bool show)
{
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
    showPopup(m_autoHideTimeout);
    if (!m_timerId) {
        m_timerId = startTimer(idleCheckInterval);
    }
}

void Applet::open(const QString &url)
{
    kDebug() << "open " << url;
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
