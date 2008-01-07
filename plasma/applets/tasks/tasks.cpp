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
#include <plasma/layouts/boxlayout.h>
#include <plasma/layouts/layoutanimator.h>

Tasks::Tasks(QObject* parent , const QVariantList &arguments)
 : Plasma::Applet(parent,arguments),
   m_dialog(0)
{
    setHasConfigurationInterface(true);
}

Tasks::~Tasks()
{
    delete m_dialog;
}

void Tasks::init()
{
    Plasma::BoxLayout *layout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight, this);
    layout->setMargin(0);
    _rootTaskGroup = new TaskGroupItem(this, this);
    _rootTaskGroup->resize(contentSize());

    // testing
        Plasma::LayoutAnimator* animator = new Plasma::LayoutAnimator;
        animator->setAutoDeleteOnRemoval(true);
        animator->setEffect(Plasma::LayoutAnimator::InsertedState,
                            Plasma::LayoutAnimator::FadeInMoveEffect);
        animator->setEffect(Plasma::LayoutAnimator::StandardState,
                            Plasma::LayoutAnimator::MoveEffect);
        animator->setEffect(Plasma::LayoutAnimator::RemovedState,
                            Plasma::LayoutAnimator::FadeOutMoveEffect);
        animator->setTimeLine(new QTimeLine(200, this));

    layout->addItem(_rootTaskGroup);

    // testing
        _rootTaskGroup->setBorderStyle(TaskGroupItem::NoBorder);
       // _rootTaskGroup->setColor( QColor(100,120,130) );
        _rootTaskGroup->setText("Root Group");

    KConfigGroup cg = config();
    _showTooltip = cg.readEntry("showTooltip", true);

    // add representations of existing running tasks
    registerWindowTasks();
    registerStartingTasks();

    // add the animator once we're initialized to avoid animating like mad on start up
    _rootTaskGroup->layout()->setAnimator(animator);
}

void Tasks::registerStartingTasks()
{
    // listen for addition and removal of starting tasks
    connect(TaskManager::TaskManager::self(), SIGNAL(startupAdded(StartupPtr)),
            this, SLOT(addStartingTask(StartupPtr)) );
    connect(TaskManager::TaskManager::self(), SIGNAL(startupRemoved(StartupPtr)),
            this, SLOT(removeStartingTask(StartupPtr)));
}

void Tasks::addStartingTask(StartupPtr task)
{
    WindowTaskItem* item = new WindowTaskItem(_rootTaskGroup, _rootTaskGroup, _showTooltip);
    item->setStartupTask(task);
    _startupTaskItems.insert(task, item);

    addItemToRootGroup(item);
}

void Tasks::removeStartingTask(StartupPtr task)
{
    if (_startupTaskItems.contains(task)) {
        removeItemFromRootGroup(_startupTaskItems[task]);
    }
}

void Tasks::registerWindowTasks()
{
    TaskManager::TaskManager *manager = TaskManager::TaskManager::self();

    TaskManager::TaskDict tasks = manager->tasks();
    QMapIterator<WId,TaskPtr> iter(tasks);

    while (iter.hasNext())
    {
        iter.next();
        addWindowTask(iter.value());
    }

    // listen for addition and removal of window tasks
    connect(TaskManager::TaskManager::self(), SIGNAL(taskAdded(TaskPtr)),
            this, SLOT(addWindowTask(TaskPtr)));
    connect(TaskManager::TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)),
            this, SLOT(removeWindowTask(TaskPtr)));
}

void Tasks::addItemToRootGroup(AbstractTaskItem *item)
{
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    _rootTaskGroup->insertTask(item);
}

void Tasks::removeItemFromRootGroup(AbstractTaskItem *item)
{
    Q_ASSERT( item );

    _rootTaskGroup->removeTask(item);

// TEMPORARY
//      scene()->removeItem(item);
//    item->deleteLater();
}

void Tasks::addWindowTask(TaskPtr task)
{
    if (!task->showInTaskbar()) {
        return;
    }

    WindowTaskItem *item = 0;
    foreach (StartupPtr startup, _startupTaskItems.keys()) {
        if (startup->matchesWindow(task->window())) {
            item = dynamic_cast<WindowTaskItem *>(_startupTaskItems.take(startup));
        }
    }

    if (!item) {
        item = new WindowTaskItem(_rootTaskGroup, _rootTaskGroup, _showTooltip);
    }

    item->setWindowTask(task);
    _windowTaskItems.insert(task,item);

    addItemToRootGroup(item);
}

void Tasks::removeWindowTask(TaskPtr task)
{
    if (_windowTaskItems.contains(task)) {
        removeItemFromRootGroup(_windowTaskItems[task]);
        _windowTaskItems.remove(task);
    }
}

void Tasks::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::LocationConstraint) {
        foreach (AbstractTaskItem *taskItem, _windowTaskItems) {
            WindowTaskItem *windowTaskItem = dynamic_cast<WindowTaskItem *>(taskItem);
            if (windowTaskItem) {
                windowTaskItem->publishIconGeometry();
            }
        }
    }
}

void Tasks::wheelEvent(QGraphicsSceneWheelEvent *e)
{
     _rootTaskGroup->cycle(e->delta());
}

void Tasks::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure Taskbar"));

        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }
    ui.showTooltip->setChecked(_showTooltip);
    m_dialog->show();
}

void Tasks::configAccepted()
{
    if (_showTooltip != (ui.showTooltip->checkState() == Qt::Checked)) {
        _showTooltip = !_showTooltip;
        foreach (AbstractTaskItem *taskItem, _windowTaskItems) {
            WindowTaskItem *windowTaskItem = dynamic_cast<WindowTaskItem *>(taskItem);
            if (windowTaskItem) {
                windowTaskItem->setShowTooltip(_showTooltip);
            }
        }
        update();

        KConfigGroup cg = config();
        cg.writeEntry("showTooltip", _showTooltip);
        emit configNeedsSaving();
    }
}


#include "tasks.moc"
