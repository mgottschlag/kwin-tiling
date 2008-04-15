/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
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

// Own
#include "tasks.h"
#include "taskgroupitem.h"
#include "windowtaskitem.h"
#include "ui_tasksConfig.h"

// KDE
#include <KDialog>

// Qt
#include <QGraphicsSceneWheelEvent>
#include <QTimeLine>
#include <QGraphicsLinearLayout>

// Plasma
#include <plasma/containment.h>
#include <plasma/theme.h>

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
 : Plasma::Applet(parent, arguments),
   m_dialog(0)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Qt::IgnoreAspectRatio);

    m_screenTimer.setSingleShot(true);
    m_screenTimer.setInterval(300);
    connect(&m_screenTimer, SIGNAL(timeout()), this, SLOT(checkScreenChange()));
    connect(Plasma::Theme::self(), SIGNAL(changed()), this, SLOT(themeRefresh()));
    setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum,QSizePolicy::DefaultType);
}

Tasks::~Tasks()
{
    delete m_dialog;
}

void Tasks::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    //layout->setMargin(0);
    if (formFactor() == Plasma::Vertical) {
        layout->setOrientation(Qt::Vertical);
    } else {
        layout->setOrientation(Qt::Horizontal);
    }
    setLayout(layout);

    m_rootTaskGroup = new TaskGroupItem(this,this);
//     m_rootTaskGroup->resize(geometry().size());
    connect(m_rootTaskGroup, SIGNAL(activated(AbstractTaskItem*)),
            this, SLOT(launchActivated()));
    layout->addItem(m_rootTaskGroup);

    m_rootTaskGroup->setBorderStyle(TaskGroupItem::NoBorder);
    // m_rootTaskGroup->setColor( QColor(100,120,130) );
    m_rootTaskGroup->setText("Root Group");

    KConfigGroup cg = config();
#ifdef TOOLTIP_MANAGER
    m_showTooltip = cg.readEntry("showTooltip", true);
#endif
    m_showOnlyCurrentDesktop = cg.readEntry("showOnlyCurrentDesktop", false);
    m_showOnlyCurrentScreen = cg.readEntry("showOnlyCurrentScreen", false);

    reconnect();

    // listen for addition and removal of window tasks
    connect(TaskManager::TaskManager::self(), SIGNAL(taskAdded(TaskPtr)),
            this, SLOT(addWindowTask(TaskPtr)));
    connect(TaskManager::TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)),
            this, SLOT(removeWindowTask(TaskPtr)));

    // listen for addition and removal of starting tasks
    connect(TaskManager::TaskManager::self(), SIGNAL(startupAdded(StartupPtr)),
            this, SLOT(addStartingTask(StartupPtr)));
    connect(TaskManager::TaskManager::self(), SIGNAL(startupRemoved(StartupPtr)),
            this, SLOT(removeStartingTask(StartupPtr)));

    // add the animator once we're initialized to avoid animating like mad on start up
    //m_rootTaskGroup->layout()->setAnimator(m_animator);
}

void Tasks::addStartingTask(StartupPtr task)
{
    WindowTaskItem* item = new WindowTaskItem(m_rootTaskGroup, m_rootTaskGroup, m_showTooltip);
    item->setStartupTask(task);
    m_startupTaskItems.insert(task, item);

    addItemToRootGroup(item);
}

void Tasks::removeStartingTask(StartupPtr task)
{
    if (m_startupTaskItems.contains(task)) {
        AbstractTaskItem *item = m_startupTaskItems.take(task);
        removeItemFromRootGroup(item);
    }
}

void Tasks::registerWindowTasks()
{
    TaskManager::TaskManager *manager = TaskManager::TaskManager::self();

    TaskManager::TaskDict tasks = manager->tasks();
    QMapIterator<WId,TaskPtr> iter(tasks);

    while (iter.hasNext()) {
        iter.next();
        addWindowTask(iter.value());
    }
}

void Tasks::addItemToRootGroup(AbstractTaskItem *item)
{
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    m_rootTaskGroup->insertTask(item);
}

void Tasks::removeItemFromRootGroup(AbstractTaskItem *item)
{
    Q_ASSERT( item );

    m_rootTaskGroup->removeTask(item);

// TEMPORARY
//      scene()->removeItem(item);
//    item->deleteLater();
}

void Tasks::addWindowTask(TaskPtr task)
{
    if (!task->showInTaskbar()) {
        return;
    }

    if (m_showOnlyCurrentDesktop && !task->isOnCurrentDesktop()) {
        return;
    }
    if (m_showOnlyCurrentScreen && !isOnMyScreen(task)) {
        return;
    }

    WindowTaskItem *item = 0;
    foreach (StartupPtr startup, m_startupTaskItems.keys()) {
        if (startup->matchesWindow(task->window())) {
            item = dynamic_cast<WindowTaskItem *>(m_startupTaskItems.take(startup));
            break;
        }
    }

    if (!item) {
        item = new WindowTaskItem(m_rootTaskGroup, m_rootTaskGroup, m_showTooltip);
    }

    item->setWindowTask(task);
    m_windowTaskItems.insert(task, item);

    addItemToRootGroup(item);
}

void Tasks::removeWindowTask(TaskPtr task)
{
    if (m_windowTaskItems.contains(task)) {
        AbstractTaskItem *item = m_windowTaskItems.take(task);
        removeItemFromRootGroup(item);
    }
}

void Tasks::removeAllWindowTasks()
{
    while (!m_windowTaskItems.isEmpty()) {
        removeItemFromRootGroup(m_windowTaskItems.take(m_windowTaskItems.constBegin().key()));
    }
}

void Tasks::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::LocationConstraint) {
        QGraphicsLinearLayout * taskslayout = dynamic_cast<QGraphicsLinearLayout *>(m_rootTaskGroup->layout());
        if (formFactor() == Plasma::Vertical) {
            taskslayout->setOrientation(Qt::Vertical);
        } else {
            taskslayout->setOrientation(Qt::Horizontal);
        }

        foreach (AbstractTaskItem *taskItem, m_windowTaskItems) {
            WindowTaskItem *windowTaskItem = dynamic_cast<WindowTaskItem *>(taskItem);
            if (windowTaskItem) {
                windowTaskItem->publishIconGeometry();
            }
        }
    }
    else if (constraints & Plasma::SizeConstraint) {
        m_rootTaskGroup->resize(effectiveSizeHint(Qt::MaximumSize));
    }
}

void Tasks::wheelEvent(QGraphicsSceneWheelEvent *e)
{
     m_rootTaskGroup->cycle(e->delta());
}

void Tasks::currentDesktopChanged(int)
{
    if (!m_showOnlyCurrentDesktop) {
        return;
    }

    removeAllWindowTasks();
    registerWindowTasks();
}

void Tasks::taskMovedDesktop(TaskPtr task)
{
    if (!m_showOnlyCurrentDesktop) {
        return;
    }

    if (!task->isOnCurrentDesktop()) {
        removeWindowTask(task);
    } else if (!m_windowTaskItems.contains(task)) {
        addWindowTask(task);
    }
}

void Tasks::windowChangedGeometry(TaskPtr task)
{
    if (!m_tasks.contains(task)) {
        m_tasks.append(task);
    }
    if (!m_screenTimer.isActive()) {
        m_screenTimer.start();
    }
}

void Tasks::checkScreenChange()
{
    foreach (TaskPtr task, m_tasks) {
        if (!isOnMyScreen(task)) {
            removeWindowTask(task);
        } else if (!m_windowTaskItems.contains(task)) {
            addWindowTask(task);
        }
    }
    m_tasks.clear();
}

bool Tasks::isOnMyScreen(TaskPtr task)
{
    Plasma::Containment* appletContainment = containment();

    if (appletContainment) {
        if (appletContainment->screen() != -1) {
            if (!TaskManager::TaskManager::isOnScreen(appletContainment->screen(),
                task->window())) {
                return false;
            }
        }
    }
    return true;
}

void Tasks::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure Taskbar"));

        QWidget *widget = new QWidget;
        m_ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);

        connect(m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    }
#ifdef TOOLTIP_MANAGER
    m_ui.showTooltip->setChecked(m_showTooltip);
#endif
    m_ui.showOnlyCurrentDesktop->setChecked(m_showOnlyCurrentDesktop);
    m_ui.showOnlyCurrentScreen->setChecked(m_showOnlyCurrentScreen);
    m_dialog->show();
}

void Tasks::configAccepted()
{
    bool changed = false;

    if (m_showOnlyCurrentDesktop != (m_ui.showOnlyCurrentDesktop->isChecked())) {
        m_showOnlyCurrentDesktop = !m_showOnlyCurrentDesktop;
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentDesktop", m_showOnlyCurrentDesktop);
        changed = true;
    }
    if (m_showOnlyCurrentScreen != (m_ui.showOnlyCurrentScreen->isChecked())) {
        m_showOnlyCurrentScreen = !m_showOnlyCurrentScreen;
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentScreen", m_showOnlyCurrentScreen);
        changed = true;
    }

    if (changed) {
        reconnect();
    }
#ifdef TOOLTIP_MANAGER
    if (m_showTooltip != (m_ui.showTooltip->checkState() == Qt::Checked)) {
        m_showTooltip = !m_showTooltip;
        foreach (AbstractTaskItem *taskItem, m_windowTaskItems) {
            WindowTaskItem *windowTaskItem = dynamic_cast<WindowTaskItem *>(taskItem);
            if (windowTaskItem) {
                windowTaskItem->setShowTooltip(m_showTooltip);
            }
        }
        KConfigGroup cg = config();
        cg.writeEntry("showTooltip", m_showTooltip);
        changed = true;
    }
#endif
    if (changed) {
        update();
        emit configNeedsSaving();
    }
}

void Tasks::reconnect()
{
    disconnect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
               this, SLOT(currentDesktopChanged(int)));
    disconnect(TaskManager::TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
               this, SLOT(taskMovedDesktop(TaskPtr)));
    if (m_showOnlyCurrentDesktop) {
        // listen to the relevant task manager signals
        connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
                this, SLOT(currentDesktopChanged(int)));
        connect(TaskManager::TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
                this, SLOT(taskMovedDesktop(TaskPtr)));
    }

    disconnect(TaskManager::TaskManager::self(), SIGNAL(windowChangedGeometry(TaskPtr)),
               this, SLOT(windowChangedGeometry(TaskPtr)));
    if (m_showOnlyCurrentScreen) {
        // listen to the relevant task manager signals
        connect(TaskManager::TaskManager::self(), SIGNAL(windowChangedGeometry(TaskPtr)),
                this, SLOT(windowChangedGeometry(TaskPtr)));
        TaskManager::TaskManager::self()->trackGeometry();
    }

    removeAllWindowTasks();
    registerWindowTasks();
}

void Tasks::themeRefresh()
{
    foreach (AbstractTaskItem *taskItem, m_windowTaskItems) {
        taskItem->update();
    }

}

#include "tasks.moc"
