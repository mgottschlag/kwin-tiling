/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "groupmanager.h"

#include <QBuffer>
#include <QList>
#include <QStack>
#include <QTimer>
#include <QUuid>

#include <KDebug>

#include "abstractsortingstrategy.h"
#include "startup.h"
#include "task.h"
#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"
#include "strategies/alphasortingstrategy.h"
#include "strategies/desktopsortingstrategy.h"
#include "strategies/programgroupingstrategy.h"
#include "strategies/manualgroupingstrategy.h"
#include "strategies/kustodiangroupingstrategy.h"
#include "strategies/manualsortingstrategy.h"
#include "launcheritem.h"

namespace TaskManager
{

class GroupManagerPrivate
{
public:
    GroupManagerPrivate(GroupManager *manager)
        : q(manager),
          sortingStrategy(GroupManager::NoSorting),
          groupingStrategy(GroupManager::NoGrouping),
          lastGroupingStrategy(GroupManager::NoGrouping),
          abstractGroupingStrategy(0),
          abstractSortingStrategy(0),
          currentScreen(-1),
          groupIsFullLimit(0),
          showOnlyCurrentDesktop(false),
          showOnlyCurrentActivity(false),
          showOnlyCurrentScreen(false),
          showOnlyMinimized(false),
          onlyGroupWhenFull(false),
          changingGroupingStrategy(false),
          readingLauncherConfig(false)
    {
    }

    /** reload all tasks from TaskManager */
    void reloadTasks();
    void actuallyReloadTasks();

    /**
    * Keep track of changes in Taskmanager
    */
    void currentDesktopChanged(int);
    void currentActivityChanged(QString);
    void taskChanged(TaskPtr, ::TaskManager::TaskChanges);
    void checkScreenChange();
    void taskDestroyed(QObject *item);
    void startupItemDestroyed(AbstractGroupableItem *);
    void checkIfFull();
    void actuallyCheckIfFull();
    bool addTask(TaskPtr);
    void removeTask(TaskPtr);
    void addStartup(StartupPtr);
    void removeStartup(StartupPtr);
    void launcherVisibilityChange();
    void checkLauncherVisibility(LauncherItem *launcher);
    void saveLauncher(LauncherItem *launcher);
    void saveLauncher(LauncherItem *launcher, KConfigGroup &group);
    void unsaveLauncher(LauncherItem *launcher);
    KConfigGroup launcherConfig(const KConfigGroup &config = KConfigGroup());

    TaskGroup *currentRootGroup();

    GroupManager *q;
    QHash<StartupPtr, TaskItem*> startupList;
    GroupManager::TaskSortingStrategy sortingStrategy;
    GroupManager::TaskGroupingStrategy groupingStrategy;
    GroupManager::TaskGroupingStrategy lastGroupingStrategy;
    AbstractGroupingStrategy *abstractGroupingStrategy;
    AbstractSortingStrategy *abstractSortingStrategy;
    int currentScreen;
    QTimer screenTimer;
    QTimer reloadTimer;
    QTimer checkIfFullTimer;
    QSet<Task *> geometryTasks;
    int groupIsFullLimit;
    QUuid configToken;

    QHash<QString, QHash<int, TaskGroup*> > rootGroups; //container for groups
    QHash<KUrl, LauncherItem *> launchers;
    int currentDesktop;
    QString currentActivity;

    bool showOnlyCurrentDesktop : 1;
    bool showOnlyCurrentActivity : 1;
    bool showOnlyCurrentScreen : 1;
    bool showOnlyMinimized : 1;
    bool onlyGroupWhenFull : 1;
    bool changingGroupingStrategy : 1;
    bool readingLauncherConfig : 1;
};


GroupManager::GroupManager(QObject *parent)
    : QObject(parent),
      d(new GroupManagerPrivate(this))
{
    connect(TaskManager::self(), SIGNAL(taskAdded(TaskPtr)), this, SLOT(addTask(TaskPtr)));
    connect(TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)), this, SLOT(removeTask(TaskPtr)));
    connect(TaskManager::self(), SIGNAL(startupAdded(StartupPtr)), this, SLOT(addStartup(StartupPtr)));
    connect(TaskManager::self(), SIGNAL(startupRemoved(StartupPtr)), this, SLOT(removeStartup(StartupPtr)));

    d->currentDesktop = TaskManager::self()->currentDesktop();
    d->currentActivity = TaskManager::self()->currentActivity();

    d->rootGroups[d->currentActivity][d->currentDesktop] = new TaskGroup(this, "RootGroup", Qt::transparent);

    d->reloadTimer.setSingleShot(true);
    d->reloadTimer.setInterval(0);
    connect(&d->reloadTimer, SIGNAL(timeout()), this, SLOT(actuallyReloadTasks()));

    d->screenTimer.setSingleShot(true);
    d->screenTimer.setInterval(100);
    connect(&d->screenTimer, SIGNAL(timeout()), this, SLOT(checkScreenChange()));

    d->checkIfFullTimer.setSingleShot(true);
    d->checkIfFullTimer.setInterval(0);
    connect(&d->checkIfFullTimer, SIGNAL(timeout()), this, SLOT(actuallyCheckIfFull()));
}

GroupManager::~GroupManager()
{
    TaskManager::self()->setTrackGeometry(false, d->configToken);
    delete d->abstractSortingStrategy;
    delete d->abstractGroupingStrategy;
    delete d;
}

TaskGroup *GroupManagerPrivate::currentRootGroup()
{
    return rootGroups[currentActivity][currentDesktop];
}

void GroupManagerPrivate::reloadTasks()
{
    reloadTimer.start();
}

void GroupManagerPrivate::actuallyReloadTasks()
{
    //kDebug() << "number of tasks available " << TaskManager::self()->tasks().size();
    QHash<WId, TaskPtr> taskList = TaskManager::self()->tasks();
    QMutableHashIterator<WId, TaskPtr> it(taskList);

    while (it.hasNext()) {
        it.next();

        if (addTask(it.value())) {
            //kDebug() << "task added " << it.value()->visibleName();
            it.remove();
        }
    }

    // Remove what remains
    it.toFront();
    while (it.hasNext()) {
        it.next();
        removeTask(it.value());
    }

    emit q->reload();
}

void GroupManagerPrivate::addStartup(StartupPtr task)
{
    //kDebug();
    if (!startupList.contains(task)) {
        TaskItem *item = new TaskItem(q, task);
        startupList.insert(task, item);
        currentRootGroup()->add(item);
        QObject::connect(item, SIGNAL(destroyed(AbstractGroupableItem*)),
                         q, SLOT(startupItemDestroyed(AbstractGroupableItem*)));
    }
}

void GroupManagerPrivate::removeStartup(StartupPtr task)
{
    //kDebug();
    if (!startupList.contains(task)) {
        kDebug() << "invalid startup task";
        return;
    }

    AbstractGroupableItem *item = startupList.take(task);
    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }
}

bool GroupManagerPrivate::addTask(TaskPtr task)
{
    //kDebug();
    /* kDebug() << task->visibleName()
             << task->visibleNameWithState()
             << task->name()
             << task->className()
             << task->classClass(); */

    if (!task->showInTaskbar()) {
        //kDebug() << "Do not show in taskbar";
        return false;
    }

    if (showOnlyCurrentScreen && !task->isOnScreen(currentScreen)) {
        //kDebug() << "Not on this screen and showOnlyCurrentScreen";
        return false;
    }

    // Should the Task be displayed ? We always display if attention is demaded
    if (!task->demandsAttention()) {
        // As the Task doesn't demand attention
        // go through all filters whether the task should be displayed or not
        if (showOnlyCurrentDesktop && !task->isOnCurrentDesktop()) {
            /* kDebug() << "Not on this desktop and showOnlyCurrentDesktop"
                     << KWindowSystem::currentDesktop() << task->desktop(); */
            return false;
        }

        if (showOnlyCurrentActivity && !task->isOnCurrentActivity()) {
            /* kDebug() << "Not on this desktop and showOnlyCurrentActivity"
                     << KWindowSystem::currentActivity() << task->desktop(); */
            return false;
        }

        if (showOnlyMinimized && !task->isMinimized()) {
            //kDebug() << "Not minimized and only showing minimized";
            return false;
        }

        NET::WindowType type = task->info().windowType(NET::NormalMask | NET::DialogMask |
                                                    NET::OverrideMask | NET::UtilityMask);
        if (type == NET::Utility) {
            //kDebug() << "skipping utility window" << task->name();
            return false;
        }

            //TODO: should we check for transiency? if so the following code can detect it.
        /*
            QHash <TaskPtr, TaskItem*>::iterator it = d->itemList.begin();

            while (it != d->itemList.end()) {
                TaskItem *item = it.value();
                if (item->task()->hasTransient(task->window())) {
                    kDebug() << "TRANSIENT TRANSIENT TRANSIENT!";
                    return flase;
                }
                ++it;
            }
        */
    }

    //Ok the Task should be displayed
    TaskItem *item = qobject_cast<TaskItem*>(currentRootGroup()->getMemberByWId(task->window()));
    if (!item) {
        // first search for an existing startuptask for this task
        QHash<StartupPtr, TaskItem *>::iterator it = startupList.begin();
        QHash<StartupPtr, TaskItem *>::iterator itEnd = startupList.end();
        while (it != itEnd) {
            if (it.key()->matchesWindow(task->window())) {
                //kDebug() << "startup task found";
                item = it.value();
                startupList.erase(it);
                QObject::disconnect(item, 0, q, 0);
                item->setTaskPointer(task);
                break;
            }
            ++it;
        }

        if (!item) {
            item = new TaskItem(q, task);
        }

        QObject::connect(task.data(), SIGNAL(destroyed(QObject*)),
                         q, SLOT(taskDestroyed(QObject*)));

        foreach (LauncherItem *launcher, launchers) {
            launcher->associateItemIfMatches(item);
        }
    }

    //Find a fitting group for the task with GroupingStrategies
    if (abstractGroupingStrategy && !task->demandsAttention()) { //do not group attention tasks
        abstractGroupingStrategy->handleItem(item);
    } else {
        currentRootGroup()->add(item);
    }

    geometryTasks.insert(task.data());

    return true;
}


void GroupManagerPrivate::removeTask(TaskPtr task)
{
    //kDebug() << "remove: " << task->visibleName();
    geometryTasks.remove(task.data());

    AbstractGroupableItem *item = currentRootGroup()->getMemberByWId(task->window());
    if (!item) {
        // this can happen if the window hasn't been caught previously,
        // of it it is an ignored type such as a NET::Utility type window
        //kDebug() << "invalid item";
        return;
    }

    foreach (LauncherItem *launcher, launchers) {
        launcher->removeItemIfAssociated(item);
    }

    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }

    //the item must exist as long as the TaskPtr does because of activate calls so don't delete the item here, it will delete itself.
}

void GroupManagerPrivate::taskDestroyed(QObject *item)
{
    Task *task = static_cast<Task*>(item);
    if (task) {
        geometryTasks.remove(task);
    }
}

void GroupManagerPrivate::startupItemDestroyed(AbstractGroupableItem *item)
{
    TaskItem *taskItem = static_cast<TaskItem*>(item);
    startupList.remove(startupList.key(taskItem));
    geometryTasks.remove(taskItem->task().data());
}

bool GroupManager::manualGroupingRequest(AbstractGroupableItem* item, TaskGroup* groupItem)
{
    if (d->abstractGroupingStrategy) {
        return d->abstractGroupingStrategy->manualGroupingRequest(item, groupItem);
    }
    return false;
}

bool GroupManager::manualGroupingRequest(ItemList items)
{
    if (d->abstractGroupingStrategy) {
            return d->abstractGroupingStrategy->manualGroupingRequest(items);
    }
    return false;
}

bool GroupManager::manualSortingRequest(AbstractGroupableItem* taskItem, int newIndex)
{
    if (d->abstractSortingStrategy) {
        return d->abstractSortingStrategy->manualSortingRequest(taskItem, newIndex);
    }
    return false;
}


GroupPtr GroupManager::rootGroup() const
{
    return d->currentRootGroup();
}

void GroupManagerPrivate::currentActivityChanged(QString newActivity)
{
    if (!showOnlyCurrentActivity || currentActivity == newActivity) {
        return;
    }

    if (!rootGroups.contains(newActivity) || !rootGroups.value(newActivity).contains(currentDesktop)) {
        kDebug() << "created new desk group";
        rootGroups[newActivity][currentDesktop] = new TaskGroup(q, "RootGroup", Qt::transparent);
        if (abstractSortingStrategy) {
            abstractSortingStrategy->handleGroup(rootGroups[newActivity][currentDesktop]);
        }
    }

    if (onlyGroupWhenFull) {
        QObject::disconnect(currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
        QObject::disconnect(currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
    }

    currentActivity = newActivity;

    foreach (LauncherItem *item, launchers) {
        if (item->shouldShow()) {
            rootGroups[currentActivity][currentDesktop]->add(item);
        } else {
            rootGroups[currentActivity][currentDesktop]->remove(item);
        }
    }

    if (onlyGroupWhenFull) {
        QObject::connect(currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
        QObject::connect(currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
    }

    reloadTasks();
}

void GroupManagerPrivate::currentDesktopChanged(int newDesktop)
{
    //kDebug();
    if (!showOnlyCurrentDesktop) {
        return;
    }

    if (currentDesktop == newDesktop) {
        return;
    }

    if (!rootGroups[currentActivity].contains(newDesktop)) {
        kDebug() << "created new desk group";
        rootGroups[currentActivity][newDesktop] = new TaskGroup(q, "RootGroup", Qt::transparent);
        if (abstractSortingStrategy) {
            abstractSortingStrategy->handleGroup(rootGroups[currentActivity][newDesktop]);
        }
    }

    if (onlyGroupWhenFull) {
        QObject::disconnect(currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
        QObject::disconnect(currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
    }

    currentDesktop = newDesktop;

    foreach (LauncherItem *item, launchers) {
        if (item->shouldShow()) {
            rootGroups[currentActivity][currentDesktop]->add(item);
        } else {
            rootGroups[currentActivity][currentDesktop]->remove(item);
        }
    }

    if (onlyGroupWhenFull) {
        QObject::connect(currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
        QObject::connect(currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), q, SLOT(checkIfFull()));
    }

    reloadTasks();
}


void GroupManagerPrivate::taskChanged(TaskPtr task, ::TaskManager::TaskChanges changes)
{
    //kDebug();
    bool takeAction = false;
    bool show = true;

    if (showOnlyCurrentDesktop && changes & ::TaskManager::DesktopChanged) {
        takeAction = true;
        show = task->isOnCurrentDesktop();
        //kDebug() << task->visibleName() << "on" << TaskManager::self()->currentDesktop();
    }

    if (showOnlyCurrentActivity && changes & ::TaskManager::ActivitiesChanged) {
        takeAction = true;
        show = task->isOnCurrentActivity();
        //kDebug() << task->visibleName() << "on" << TaskManager::self()->currentDesktop();
    }

    if (showOnlyMinimized && changes & ::TaskManager::StateChanged) {
        //TODO: wouldn't it be nice to get notification of JUST minimization?
        takeAction = true;
        show = task->isMinimized();
    }

    if (showOnlyCurrentScreen && changes & ::TaskManager::GeometryChanged) {
        geometryTasks.insert(task.data());

        if (!screenTimer.isActive()) {
            screenTimer.start();
        }
    }

    if (changes & ::TaskManager::AttentionChanged) {
        // we show tasks anyway if they demand attention
        // so whenever our state changes ... try to re-adjust it
        takeAction = true;
        show = true;
    }

    if (!takeAction) {
        return;
    }

    show = show && (!showOnlyCurrentScreen || task->isOnScreen(currentScreen));

    if (show) {
        //kDebug() << "add(task);";
        addTask(task);
    } else {
        //kDebug() << "remove(task);";
        removeTask(task);
    }
}

void GroupManager::setScreen(int screen)
{
    //kDebug() << "new Screen: " << screen;
    d->currentScreen = screen;
    d->reloadTasks();
}


void GroupManagerPrivate::checkScreenChange()
{
    //kDebug();
    foreach (Task *task, geometryTasks) {
        if (task->isOnScreen(currentScreen)) {
            addTask(TaskPtr(task));
        } else {
            removeTask(TaskPtr(task));
        }
    }
}

void GroupManager::reconnect()
{
    //kDebug();
    disconnect(TaskManager::self(), SIGNAL(desktopChanged(int)),
               this, SLOT(currentDesktopChanged(int)));
    disconnect(TaskManager::self(), SIGNAL(activityChanged(QString)),
               this, SLOT(currentActivityChanged(QString)));
    disconnect(TaskManager::self(), SIGNAL(windowChanged(TaskPtr,::TaskManager::TaskChanges)),
               this, SLOT(taskChanged(TaskPtr,::TaskManager::TaskChanges)));

    if (d->showOnlyCurrentDesktop || d->showOnlyMinimized || d->showOnlyCurrentScreen || d->showOnlyCurrentActivity) {
        // listen to the relevant task manager signals
        if (d->showOnlyCurrentDesktop) {
            connect(TaskManager::self(), SIGNAL(desktopChanged(int)),
                    this, SLOT(currentDesktopChanged(int)));
        }
        if (d->showOnlyCurrentActivity) {
            connect(TaskManager::self(), SIGNAL(activityChanged(QString)),
                    this, SLOT(currentActivityChanged(QString)));
        }

        connect(TaskManager::self(), SIGNAL(windowChanged(TaskPtr,::TaskManager::TaskChanges)),
                this, SLOT(taskChanged(TaskPtr,::TaskManager::TaskChanges)));
    }

    TaskManager::self()->setTrackGeometry(d->showOnlyCurrentScreen, d->configToken);

    if (!d->showOnlyCurrentScreen) {
        d->geometryTasks.clear();
    }

    d->reloadTasks();
}

KConfigGroup GroupManager::config() const
{
    return KConfigGroup();
}

bool GroupManager::addLauncher(const KUrl &url, QIcon icon, QString name, QString genericName)
{
    if (url.isEmpty()) {
        return false;
    }

    LauncherItem *launcher = d->launchers.value(url); // Do not insert launchers twice
    if (!launcher) {
        launcher = new LauncherItem(d->currentRootGroup(), url);

        if (!launcher->isValid()) {
            delete launcher;
            return false;
        }

        if (!icon.isNull()) {
            launcher->setIcon(icon);
        }

        if (!name.isEmpty()) {
            launcher->setName(name);
        }

        if (!genericName.isEmpty()) {
            launcher->setGenericName(genericName);
        }

        QStack<TaskGroup *> groups;
        groups.push(d->currentRootGroup());
        while (!groups.isEmpty()) {
            TaskGroup *group = groups.pop();

            foreach (AbstractGroupableItem *item, group->members()) {
                if (item->itemType() == GroupItemType) {
                    groups.push(static_cast<TaskGroup *>(item));
                } else {
                    launcher->associateItemIfMatches(item);
                }
            }
        }

        d->launchers.insert(url, launcher);
        connect(launcher, SIGNAL(show(bool)), this, SLOT(launcherVisibilityChange()));
        d->checkLauncherVisibility(launcher);
        d->saveLauncher(launcher);
    }

    return launcher;
}

void GroupManager::removeLauncher(const KUrl &url)
{
    LauncherItem *launcher = d->launchers.value(url);
    if (!launcher) {
        return;
    }

    d->launchers.remove(url);

    typedef QHash<int,TaskGroup*> Metagroup;
    foreach (Metagroup metagroup, d->rootGroups) {
        foreach (TaskGroup *rootGroup, metagroup) {
            rootGroup->remove(launcher);
        }
    }

    d->unsaveLauncher(launcher);
    launcher->deleteLater();
}

void GroupManagerPrivate::launcherVisibilityChange()
{
    checkLauncherVisibility(qobject_cast<LauncherItem *>(q->sender()));
}

void GroupManagerPrivate::checkLauncherVisibility(LauncherItem *launcher)
{
    if (!launcher) {
        return;
    }

    if (launcher->shouldShow()) {
        rootGroups[currentActivity][currentDesktop]->add(launcher);
    } else {
        rootGroups[currentActivity][currentDesktop]->remove(launcher);
    }
}

bool GroupManager::launcherExists(const KUrl &url) const
{
    return d->launchers.value(url);
}

void GroupManager::readLauncherConfig(const KConfigGroup &cg)
{
    KConfigGroup conf = d->launcherConfig(cg);
    if (!conf.isValid()) {
        return;
    }

    // prevents re-writing the results out
    d->readingLauncherConfig = true;
    QSet<KUrl> urls;
    foreach (const QString &key, conf.keyList()) {
        QStringList item = conf.readEntry(key, QStringList());
        if (item.length() >= 4) {
            KUrl url(item.at(0));
            KIcon icon;
            if (!item.at(1).isEmpty()) {
                icon = KIcon(item.at(1));
            } else if (item.length() >= 5) {
                QPixmap pixmap;
                QByteArray bytes = QByteArray::fromBase64(item.at(4).toAscii());
                pixmap.loadFromData(bytes);
                icon.addPixmap(pixmap);
            }
            QString name(item.at(2));
            QString genericName(item.at(3));

            if (addLauncher(url, icon, name, genericName)) {
                urls << url;
            }
        }
    }

    d->readingLauncherConfig = false;

    // a bit paranoiac, perhaps, but we check the removals first and then
    // remove the launchers after that scan because Qt's iterators operate
    // on a copy of the list and/or don't like multiple iterators messing
    // with the same list. we might get away with just calling removeLauncher
    // immediately without the removals KUrl::List, but this is known safe
    // and not a performance bottleneck
    KUrl::List removals;
    foreach (LauncherItem *launcher, d->launchers) {
        if (!urls.contains(launcher->launcherUrl())) {
            removals << launcher->launcherUrl();
        }
    }

    foreach (const KUrl &url, removals) {
        removeLauncher(url);
    }
}

void GroupManager::exportLauncherConfig(const KConfigGroup &cg)
{
    KConfigGroup conf = d->launcherConfig(cg);
    if (!conf.isValid()) {
        return;
    }

    foreach (LauncherItem *launcher, d->launchers) {
        d->saveLauncher(launcher, conf);
    }
}

KConfigGroup GroupManagerPrivate::launcherConfig(const KConfigGroup &config)
{
    KConfigGroup cg = config.isValid() ? config : q->config();
    if (!cg.isValid()) {
        return cg;
    }

    return KConfigGroup(&cg, "Launchers");
}

void GroupManagerPrivate::saveLauncher(LauncherItem *launcher)
{
    if (readingLauncherConfig) {
        return;
    }

    KConfigGroup cg = launcherConfig();
    if (!cg.isValid()) {
        return;
    }

    saveLauncher(launcher, cg);
    emit q->configChanged();
}

void GroupManagerPrivate::saveLauncher(LauncherItem *launcher, KConfigGroup &cg)
{
    QVariantList launcherProperties;
    launcherProperties.append(launcher->launcherUrl().url());
    launcherProperties.append(launcher->icon().name());
    launcherProperties.append(launcher->name());
    launcherProperties.append(launcher->genericName());

    if (launcher->icon().name().isEmpty()) {
        QPixmap pixmap = launcher->icon().pixmap(QSize(64,64));
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        launcherProperties.append(bytes.toBase64());
    }

    cg.writeEntry(launcher->name(), launcherProperties);
}

void GroupManagerPrivate::unsaveLauncher(LauncherItem *launcher)
{
    KConfigGroup cg = launcherConfig();
    if (!cg.isValid()) {
        return;
    }

    cg.deleteEntry(launcher->name());
    emit q->configChanged();
}

bool GroupManager::onlyGroupWhenFull() const
{
    return d->onlyGroupWhenFull;
}

void GroupManager::setOnlyGroupWhenFull(bool onlyGroupWhenFull)
{
    //kDebug() << onlyGroupWhenFull;
    if (d->onlyGroupWhenFull == onlyGroupWhenFull) {
        return;
    }

    d->onlyGroupWhenFull = onlyGroupWhenFull;

    disconnect(d->currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
    disconnect(d->currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), this, SLOT(checkIfFull()));

    if (onlyGroupWhenFull) {
        connect(d->currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
        connect(d->currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
        d->checkIfFull();
    }
}

void GroupManager::setFullLimit(int limit)
{
    //kDebug() << limit;
    d->groupIsFullLimit = limit;
    if (d->onlyGroupWhenFull) {
        d->checkIfFull();
    }
}

void GroupManagerPrivate::checkIfFull()
{
    // Start a timer so that if we have been triggered by a layouting
    // we give time for it to finish up instead of starting a new one
    // right away.
    checkIfFullTimer.start();
}

void GroupManagerPrivate::actuallyCheckIfFull()
{
    //kDebug();
    if (!onlyGroupWhenFull ||
        groupingStrategy != GroupManager::ProgramGrouping ||
        changingGroupingStrategy) {
        return;
    }

    if (currentRootGroup()->totalSize() >= groupIsFullLimit) {
        if (!abstractGroupingStrategy) {
            geometryTasks.clear();
            q->setGroupingStrategy(GroupManager::ProgramGrouping);
        }
    } else if (abstractGroupingStrategy) {
        geometryTasks.clear();
        q->setGroupingStrategy(GroupManager::NoGrouping);
        //let the visualization think we still use the programGrouping
        groupingStrategy = GroupManager::ProgramGrouping;
    }
}

bool GroupManager::showOnlyCurrentScreen() const
{
    return d->showOnlyCurrentScreen;
}

void GroupManager::setShowOnlyCurrentScreen(bool showOnlyCurrentScreen)
{
    d->showOnlyCurrentScreen = showOnlyCurrentScreen;
    reconnect();
}

bool GroupManager::showOnlyCurrentDesktop() const
{
    return d->showOnlyCurrentDesktop;
}

void GroupManager::setShowOnlyCurrentDesktop(bool showOnlyCurrentDesktop)
{
    d->showOnlyCurrentDesktop = showOnlyCurrentDesktop;
    reconnect();
}

bool GroupManager::showOnlyCurrentActivity() const
{
    return d->showOnlyCurrentActivity;
}

void GroupManager::setShowOnlyCurrentActivity(bool showOnlyCurrentActivity)
{
    d->showOnlyCurrentActivity = showOnlyCurrentActivity;
    reconnect();
}

bool GroupManager::showOnlyMinimized() const
{
    return d->showOnlyMinimized;
}

void GroupManager::setShowOnlyMinimized(bool showOnlyMinimized)
{
    d->showOnlyMinimized = showOnlyMinimized;
    reconnect();
}

GroupManager::TaskSortingStrategy GroupManager::sortingStrategy() const
{
    return d->sortingStrategy;
}

AbstractSortingStrategy* GroupManager::taskSorter() const
{
    return d->abstractSortingStrategy;
}

void GroupManager::setSortingStrategy(TaskSortingStrategy sortOrder)
{
    //kDebug() << sortOrder;

    if (d->abstractSortingStrategy) {
        if (d->abstractSortingStrategy->type() == sortOrder) {
            return;
        }

        d->abstractSortingStrategy->deleteLater();
        d->abstractSortingStrategy = 0;
    }

    switch (sortOrder) {
        case ManualSorting:
            d->abstractSortingStrategy = new ManualSortingStrategy(this);
            break;

        case AlphaSorting:
            d->abstractSortingStrategy = new AlphaSortingStrategy(this);
            break;

        case DesktopSorting:
            d->abstractSortingStrategy = new DesktopSortingStrategy(this);
            break;

        case NoSorting: //manual and no grouping result both in non automatic grouping
            break;

        default:
            kDebug() << "Invalid Strategy";
    }
    if (d->abstractSortingStrategy) {
        typedef QHash<int,TaskGroup*> Metagroup;
        foreach (Metagroup metagroup, d->rootGroups) {
            foreach (TaskGroup *group, metagroup) {
                d->abstractSortingStrategy->handleGroup(group);
            }
        }
    }

    d->sortingStrategy = sortOrder;
    reconnect();
}

GroupManager::TaskGroupingStrategy GroupManager::groupingStrategy() const
{
    return d->groupingStrategy;
}

AbstractGroupingStrategy* GroupManager::taskGrouper() const
{
    return d->abstractGroupingStrategy;
}


void GroupManager::setGroupingStrategy(TaskGroupingStrategy strategy)
{
    if (d->changingGroupingStrategy ||
        (d->abstractGroupingStrategy && d->abstractGroupingStrategy->type() == strategy)) {
        return;
    }

    d->changingGroupingStrategy = true;

    //kDebug() << strategy << kBacktrace();
    if (d->onlyGroupWhenFull) {
        disconnect(d->currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
        disconnect(d->currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
    }

    if (d->abstractGroupingStrategy) {
        disconnect(d->abstractGroupingStrategy, 0, this, 0);
        d->abstractGroupingStrategy->destroy();
        d->abstractGroupingStrategy = 0;
    }

    switch (strategy) {
        case ManualGrouping:
            d->abstractGroupingStrategy = new ManualGroupingStrategy(this);
            break;

        case ProgramGrouping:
            d->abstractGroupingStrategy = new ProgramGroupingStrategy(this);
            break;

        case KustodianGrouping:
            d->abstractGroupingStrategy = new KustodianGroupingStrategy(this);
            break;

        case NoGrouping:
            break;

        default:
            kDebug() << "Strategy not implemented";
    }

    d->groupingStrategy = strategy;

    d->actuallyReloadTasks();

    if (d->onlyGroupWhenFull) {
        connect(d->currentRootGroup(), SIGNAL(itemAdded(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
        connect(d->currentRootGroup(), SIGNAL(itemRemoved(AbstractGroupableItem *)), this, SLOT(checkIfFull()));
    }

    d->changingGroupingStrategy = false;
}

} // TaskManager namespace

#include "groupmanager.moc"
