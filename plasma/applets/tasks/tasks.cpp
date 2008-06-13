/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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
#include "windowtaskitem.h"
#include "ui_tasksConfig.h"

// KDE
#include <KConfigDialog>

// Qt
#include <QGraphicsSceneWheelEvent>
#include <QTimeLine>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>

// Plasma
#include <plasma/containment.h>
#include <plasma/panelsvg.h>
#include <plasma/theme.h>

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
 : Plasma::Applet(parent, arguments),
   m_activeTask(0),
   m_taskItemBackground(0),
   m_taskAlphaPixmap(0),
   m_colorScheme(0),
   m_leftMargin(0),
   m_topMargin(0),
   m_rightMargin(0),
   m_bottomMargin(0)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_screenTimer.setSingleShot(true);
    m_screenTimer.setInterval(300);
    connect(&m_screenTimer, SIGNAL(timeout()), this, SLOT(checkScreenChange()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeRefresh()));
}

Tasks::~Tasks()
{
}

void Tasks::init()
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    //like in Qt's designer
    //TODO : Qt's bug??
    setMaximumSize(INT_MAX,INT_MAX);

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    //TODO : Qt's bug??
    m_layout->setMaximumSize(INT_MAX,INT_MAX);
    m_layout->addStretch();

    if (formFactor() == Plasma::Vertical) {
        m_layout->setOrientation(Qt::Vertical);
    } else {
        m_layout->setOrientation(Qt::Horizontal);
    }
    setLayout(m_layout);

    KConfigGroup cg = config();
#ifdef TOOLTIP_MANAGER
    m_showTooltip = cg.readEntry("showTooltip", true);
#endif
    m_showOnlyCurrentDesktop = cg.readEntry("showOnlyCurrentDesktop", false);
    m_showOnlyCurrentScreen = cg.readEntry("showOnlyCurrentScreen", false);

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

    reconnect();
}

void Tasks::addStartingTask(StartupPtr task)
{
    WindowTaskItem* item = new WindowTaskItem(this, m_showTooltip);
    item->setStartupTask(task);
    m_startupTaskItems.insert(task, item);
    insertItemBeforeSpacer(item);
}

void Tasks::removeStartingTask(StartupPtr task)
{
    if (m_startupTaskItems.contains(task)) {
        WindowTaskItem *item = m_startupTaskItems.take(task);
        m_layout->removeItem(item);
        scene()->removeItem(item);
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
    foreach (const StartupPtr &startup, m_startupTaskItems.keys()) {
        if (startup->matchesWindow(task->window())) {
            item = dynamic_cast<WindowTaskItem *>(m_startupTaskItems.take(startup));
            break;
        }
    }

    if (!item) {
        item = new WindowTaskItem(this, m_showTooltip);
        insertItemBeforeSpacer(item);
    }
    item->setWindowTask(task);
    m_windowTaskItems.insert(task, item);

    //if active initialize m_activeTask at the last item inserted
    if (task->isActive()) {
        m_activeTask = m_windowTaskItems.find(task);
    } else if (m_windowTaskItems.count() == 1) {
        m_activeTask = m_windowTaskItems.begin();
    }

    connect(item, SIGNAL(activated(WindowTaskItem*)),
            this, SLOT(updateActive(WindowTaskItem*)));
}

void Tasks::removeWindowTask(TaskPtr task)
{
    if (m_windowTaskItems.contains(task)) {
        WindowTaskItem *item = m_windowTaskItems.take(task);
        m_layout->removeItem(item);
        scene()->removeItem(item);
        item->deleteLater();
        m_activeTask = m_windowTaskItems.end();
    }
}

void Tasks::removeAllWindowTasks()
{
    QHash<TaskPtr,WindowTaskItem*>::iterator it = m_windowTaskItems.begin();

    while (it != m_windowTaskItems.end()) {
        WindowTaskItem *item = it.value();
        m_layout->removeItem(item);
        scene()->removeItem(item);
        item->deleteLater();
        ++it;
    }

    m_windowTaskItems.clear();
    m_activeTask = m_windowTaskItems.end();
}

void Tasks::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::LocationConstraint) {
        if (formFactor() == Plasma::Vertical) {
            m_layout->setOrientation(Qt::Vertical);
        } else {
            m_layout->setOrientation(Qt::Horizontal);
        }
    }
}

Plasma::PanelSvg* Tasks::itemBackground()
{
    if (!m_taskItemBackground) {
        QString tasksThemePath = Plasma::Theme::defaultTheme()->imagePath("widgets/tasks");

        if (!tasksThemePath.isEmpty()) {
            m_taskItemBackground = new Plasma::PanelSvg(this);
            m_taskItemBackground->setImagePath(tasksThemePath);
            m_taskItemBackground->setCacheAllRenderedPanels(true);
        }
    }

    return m_taskItemBackground;
}

QPixmap *Tasks::taskAlphaPixmap()
{
    return m_taskAlphaPixmap;
}

void Tasks::resizeItemBackground(const QSizeF &size)
{
    if (!m_taskItemBackground) {
        itemBackground();

        if (!m_taskItemBackground) {
            return;
        }
    }

    if (m_taskItemBackground->panelSize() == size) {
        return;
    }

    m_taskItemBackground->clearCache();
    m_taskItemBackground->resizePanel(size);
    //get the margins now
    m_taskItemBackground->getMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
    //if the task height is too little reset the top and bottom margins
    if (size.height() - m_topMargin - m_bottomMargin < KIconLoader::SizeSmall) {
        m_topMargin = 0;
        m_bottomMargin = 0;
    }
}

KColorScheme *Tasks::colorScheme()
{
    if (!m_colorScheme) {
        m_colorScheme = new KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    }

    return m_colorScheme;
}

void Tasks::updateActive(WindowTaskItem *task)
{
    m_activeTask = m_windowTaskItems.find(task->windowTask());
}

void Tasks::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    //zero or one tasks don't cycle
    if (m_windowTaskItems.count() < 2) {
        return;
    }

    // if it's invalid move to start
    if (m_activeTask ==  m_windowTaskItems.constEnd()) {
        m_activeTask = m_windowTaskItems.begin();
    //mouse wheel down
    } else if (e->delta() < 0) {
        m_activeTask++;

        if (m_activeTask == m_windowTaskItems.constEnd()) {
            m_activeTask = m_windowTaskItems.begin();
        }
    //mouse wheel up
    } else {
        if (m_activeTask == m_windowTaskItems.constBegin()) {
            m_activeTask = m_windowTaskItems.end();
        }

        m_activeTask--;
    }

    m_activeTask.value()->activate();
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
    foreach (const TaskPtr &task, m_tasks) {
        if (!isOnMyScreen(task)) {
            removeWindowTask(task);
        } else if (!m_windowTaskItems.contains(task)) {
            addWindowTask(task);
        }
    }
    m_tasks.clear();
}

void Tasks::insertItemBeforeSpacer(QGraphicsWidget * item)
{
    if (m_layout->count() == 1) {
        m_layout->insertItem(0,item);
    }
    else {
        m_layout->insertItem(m_layout->count()-1,item);
    }
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

void Tasks::createConfigurationInterface(KConfigDialog *parent)
{
     QWidget *widget = new QWidget;
     m_ui.setupUi(widget);
     parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
     connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
     connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
     parent->addPage(widget, parent->windowTitle(), icon());

#ifdef TOOLTIP_MANAGER
    m_ui.showTooltip->setChecked(m_showTooltip);
#else
    m_ui.showTooltip->hide();
#endif
    m_ui.showOnlyCurrentDesktop->setChecked(m_showOnlyCurrentDesktop);
    m_ui.showOnlyCurrentScreen->setChecked(m_showOnlyCurrentScreen);
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
    delete m_taskItemBackground;
    m_taskItemBackground = 0;

    delete m_colorScheme;
    m_colorScheme = 0;

    foreach (WindowTaskItem *taskItem, m_windowTaskItems) {
        taskItem->update();
    }
}

K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#include "tasks.moc"
