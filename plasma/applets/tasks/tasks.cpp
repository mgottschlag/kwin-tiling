/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight                                   *
 *   robertknight@gmail.com                                                *
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

// Plasma
#include <plasma/containment.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/layouts/layoutanimator.h>

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
 : Plasma::Applet(parent, arguments),
   m_dialog(0)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Qt::IgnoreAspectRatio);
    setContentSize(500, 48);
}

Tasks::~Tasks()
{
    delete m_dialog;
}

void Tasks::init()
{
    Plasma::BoxLayout *layout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight, this);
    layout->setMargin(0);
    m_rootTaskGroup = new TaskGroupItem(this, this);
    m_rootTaskGroup->resize(contentSize());
    connect(m_rootTaskGroup, SIGNAL(activated(AbstractTaskItem*)),
            this, SLOT(launchActivated()));

    // set up the animator used in the root item
    // TODO: this really should be moved to TaskGroupItem
    Plasma::LayoutAnimator* animator = new Plasma::LayoutAnimator;
    animator->setAutoDeleteOnRemoval(true);
    animator->setEffect(Plasma::LayoutAnimator::InsertedState,
                        Plasma::LayoutAnimator::FadeInMoveEffect);
    animator->setEffect(Plasma::LayoutAnimator::StandardState,
                        Plasma::LayoutAnimator::MoveEffect);
    animator->setEffect(Plasma::LayoutAnimator::RemovedState,
                        Plasma::LayoutAnimator::FadeOutMoveEffect);
    animator->setTimeLine(new QTimeLine(200, this));

    layout->addItem(m_rootTaskGroup);

    m_rootTaskGroup->setBorderStyle(TaskGroupItem::NoBorder);
    // m_rootTaskGroup->setColor( QColor(100,120,130) );
    m_rootTaskGroup->setText("Root Group");

    KConfigGroup cg = config();
    m_showTooltip = cg.readEntry("showTooltip", true);
    m_showOnlyCurrentDesktop = cg.readEntry("showOnlyCurrentDesktop", false);

    if (m_showOnlyCurrentDesktop) {
        // listen to the relevant task manager signals
        connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
                this, SLOT(currentDesktopChanged(int)));
        connect(TaskManager::TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
                this, SLOT(taskMovedDesktop(TaskPtr)));
    }

    // add representations of existing running tasks
    registerWindowTasks();
    registerStartingTasks();

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
    m_rootTaskGroup->layout()->setAnimator(animator);
}

QList<QAction*> Tasks::contextActions()
{
    // What we do here is to return the context-actions our parent containment does
    // provide to us. This allows us to e.g. display also the "Configure Panel" action
    // the panelcontainment does provide if we right-click on the task-applet that is
    // embedded within the panel.
    return containment() ? containment()->contextActions() : QList<QAction*>();
}

void Tasks::registerStartingTasks()
{
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
        if (formFactor() == Plasma::Vertical) {
            m_rootTaskGroup->setDirection(Plasma::BoxLayout::TopToBottom);
        } else {
            m_rootTaskGroup->setDirection(Plasma::BoxLayout::LeftToRight);
        }

        foreach (AbstractTaskItem *taskItem, m_windowTaskItems) {
            WindowTaskItem *windowTaskItem = dynamic_cast<WindowTaskItem *>(taskItem);
            if (windowTaskItem) {
                windowTaskItem->publishIconGeometry();
            }
        }
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

    m_ui.showTooltip->setChecked(m_showTooltip);
    m_ui.showOnlyCurrentDesktop->setChecked(m_showOnlyCurrentDesktop);
    m_dialog->show();
}

void Tasks::configAccepted()
{
    bool changed = false;

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

    if (m_showOnlyCurrentDesktop != (m_ui.showOnlyCurrentDesktop->isChecked())) {
        m_showOnlyCurrentDesktop = !m_showOnlyCurrentDesktop;

        if (m_showOnlyCurrentDesktop) {
            // listen to the relevant task manager signals
            connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
                    this, SLOT(currentDesktopChanged(int)));
            connect(TaskManager::TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
                    this, SLOT(taskMovedDesktop(TaskPtr)));
        } else {
            disconnect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
                       this, SLOT(currentDesktopChanged(int)));
            disconnect(TaskManager::TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
                       this, SLOT(taskMovedDesktop(TaskPtr)));
        }

        removeAllWindowTasks();
        registerWindowTasks();

        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentDesktop", m_showOnlyCurrentDesktop);
        changed = true;
    }

    if (changed) {
        update();
        emit configNeedsSaving();
    }
}


#include "tasks.moc"
