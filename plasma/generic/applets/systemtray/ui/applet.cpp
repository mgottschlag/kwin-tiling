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

#include "../core/manager.h"
#include "../core/notification.h"
#include "../core/completedjobnotification.h"
#include "jobwidget.h"
#include "jobtotalswidget.h"
#include "notificationscroller.h"
#include "notificationstack.h"
#include "taskarea.h"
#include "stackdialog.h"

namespace SystemTray
{


K_EXPORT_PLASMA_APPLET(systemtray, Applet)


Manager *Applet::s_manager = 0;
int Applet::s_managerUsage = 0;

Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      m_taskArea(0),
      m_background(0),
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

    foreach (Notification *notification, s_manager->notifications()) {
        // we don't want a destroyed managed after the destruction of manager
        disconnect(notification, 0, this, 0);
    }

    clearAllCompletedJobs();

    --s_managerUsage;
    if (s_managerUsage < 1) {
        delete s_manager;
        s_manager = 0;
        s_managerUsage = 0;
    }
    delete m_notificationStackDialog;
}

void Applet::init()
{
    connect(s_manager, SIGNAL(taskAdded(SystemTray::Task*)),
            m_taskArea, SLOT(addTask(SystemTray::Task*)));
    //TODO: we re-add the task when it changed: slightly silly!
    connect(s_manager, SIGNAL(taskChanged(SystemTray::Task*)),
            m_taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(s_manager, SIGNAL(taskRemoved(SystemTray::Task*)),
            m_taskArea, SLOT(removeTask(SystemTray::Task*)));

    connect(m_taskArea, SIGNAL(sizeHintChanged(Qt::SizeHint)),
            this, SLOT(propogateSizeHintChange(Qt::SizeHint)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(checkSizes()));

    extender()->setEmptyExtenderMessage(i18n("No notifications and no jobs"));
    extender()->setWindowFlags(Qt::X11BypassWindowManagerHint);

    //destroy any item in the systray, that doesn't belong to the completedJobsGroup, since running
    //jobs and notifications can't really survive reboots anyways
    foreach (Plasma::ExtenderItem *item, extender()->attachedItems()) {
        if (!item->isGroup() && (item->group() != extender()->group("completedJobsGroup"))) {
            item->destroy();
        }
    }

    configChanged();
}

void Applet::configChanged()
{
    KConfigGroup cg = config();

    const QStringList hiddenTypes = cg.readEntry("hidden", QStringList());
    const QStringList alwaysShownTypes = cg.readEntry("alwaysShown", QStringList());
    m_taskArea->setHiddenTypes(hiddenTypes);
    m_taskArea->setAlwaysShownTypes(alwaysShownTypes);

    m_shownCategories.clear();

    if (cg.readEntry("ShowApplicationStatus", true)) {
        m_shownCategories.insert(Task::ApplicationStatus);
    }

    if (cg.readEntry("ShowCommunications", true)) {
        m_shownCategories.insert(Task::Communications);
    }

    if (cg.readEntry("ShowSystemServices", true)) {
        m_shownCategories.insert(Task::SystemServices);
    }

    if (cg.readEntry("ShowHardware", true)) {
        m_shownCategories.insert(Task::Hardware);
    }

    m_shownCategories.insert(Task::UnknownCategory);

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


    s_manager->loadApplets(cg, this);
    m_taskArea->syncTasks(s_manager->tasks());
    initExtenderTask(createExtenderTask);
    checkSizes();
    setTaskAreaGeometry();
}

void Applet::initExtenderTask(bool create)
{
    if (create) {
        extender(); // make sure it exists
        m_taskArea->addTask(s_manager->extenderTask());
    } else if (s_manager->extenderTask(false)) {
        m_taskArea->removeTask(s_manager->extenderTask());
        QGraphicsWidget *widget = s_manager->extenderTask()->widget(this, false);
        if (widget) {
            widget->deleteLater();
        }
    }
}

void Applet::syncNotificationBarNeeded()
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

    if (constraints & Plasma::ImmutableConstraint) {
        if (m_plasmoidTasksInterface) {
            bool visible = (immutability() == Plasma::UserImmutable);
            m_plasmoidTasksUi.applets->setEnabled(immutability() == Plasma::Mutable);
            m_plasmoidTasksUi.unlockLabel->setVisible(visible);
            m_plasmoidTasksUi.unlockButton->setVisible(visible);
        }
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

        QAction *unlockAction = 0;
        if (containment() && containment()->corona()) {
            unlockAction = containment()->corona()->action("lock widgets");
        }
        if (unlockAction) {
            disconnect(m_plasmoidTasksUi.unlockButton, SIGNAL(clicked()), unlockAction, SLOT(trigger()));
            connect(m_plasmoidTasksUi.unlockButton, SIGNAL(clicked()), unlockAction, SLOT(trigger()));
        }


        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(m_notificationInterface.data(), i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));
        parent->addPage(m_autoHideInterface.data(), i18n("Auto Hide"), "window-suppressed");
        parent->addPage(m_plasmoidTasksInterface.data(), i18n("Plasma Widgets"), "plasma");

        bool visible = (immutability() == Plasma::UserImmutable);
        m_plasmoidTasksUi.applets->setEnabled(immutability() == Plasma::Mutable);
        m_plasmoidTasksUi.unlockLabel->setVisible(visible);
        m_plasmoidTasksUi.unlockButton->setVisible(visible);
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
        itemCombo->addItem(i18nc("Item is always visible in the systray", "Always Visible"));

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

    KConfigGroup cg = config();
    cg.writeEntry("hidden", hiddenTypes);
    cg.writeEntry("alwaysShown", alwaysShownTypes);
    cg.writeEntry("AutoHidePopup", m_autoHideUi.autoHide->isChecked());

    KConfigGroup globalCg = globalConfig();
    globalCg.writeEntry("ShowJobs", m_notificationUi.showJobs->isChecked());
    globalCg.writeEntry("ShowNotifications", m_notificationUi.showNotifications->isChecked());
    globalCg.writeEntry("ShowApplicationStatus", m_notificationUi.showApplicationStatus->isChecked());
    globalCg.writeEntry("ShowCommunications", m_notificationUi.showCommunications->isChecked());
    globalCg.writeEntry("ShowSystemServices", m_notificationUi.showSystemServices->isChecked());
    globalCg.writeEntry("ShowHardware", m_notificationUi.showHardware->isChecked());

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
    syncNotificationBarNeeded();

    //At this point we are sure the pointer is valid
    m_notificationScroller->addNotification(notification);

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
        m_notificationStackDialog->show();
        Plasma::WindowEffects::slideWindow(m_notificationStackDialog, location());
    }
}

void Applet::addJob(Job *job)
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

        KWindowSystem::setState(m_standaloneJobSummaryDialog->winId(), NET::SkipTaskbar|NET::SkipPager);
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
        KWindowSystem::raiseWindow(m_standaloneJobSummaryDialog->winId());
        Plasma::WindowEffects::slideWindow(m_standaloneJobSummaryDialog, location());
    }
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

    if (extenderItem->config().readEntry("type", "") == "job") {
        extenderItem->setWidget(new JobWidget(0, extenderItem));
    //unknown type, this should never happen
    } else {
        extenderItem->destroy();
    }

}

void Applet::popupEvent(bool show)
{
    //decide about showing the tiny progressbar or not
    if (m_standaloneJobSummaryDialog) {
        if (show || !s_manager->jobs().isEmpty()) {
            m_standaloneJobSummaryDialog->setVisible(!show);
            if (!show) {
                KWindowSystem::raiseWindow(m_standaloneJobSummaryDialog->winId());
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
    //finished all jobs? hide the mini progressbar
    if (m_standaloneJobSummaryDialog && s_manager->jobs().isEmpty()) {
        m_standaloneJobSummaryDialog->hide();
    }

    //create a fake notification
    CompletedJobNotification *notification = new CompletedJobNotification(this);
    notification->setJob(job);
    s_manager->addNotification(notification);
}

void Applet::open(const QString &url)
{
    //kDebug() << "open " << url;
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
