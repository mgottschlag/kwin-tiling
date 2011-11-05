/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>
Copyright (C) 2011 Craig Drummond <craig@kde.org>

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

// Own
#include "taskitem.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KDesktopFile>
#include <KService>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <ksysguard/processcore/processes.h>
#include <ksysguard/processcore/process.h>

#include <QtCore/QFile>
#include <QtCore/QTimer>

#include "groupmanager.h"

namespace TaskManager
{


class TaskItem::Private
{
public:
    Private()
        : startupTask(0)
        , checkedForLauncher(false) {
    }

    QWeakPointer<Task> task;
    StartupPtr startupTask;
    KUrl launcherUrl;
    bool checkedForLauncher;
    QString taskName;
};


TaskItem::TaskItem(QObject *parent, TaskPtr task)
    : AbstractGroupableItem(parent),
      d(new Private)
{
    setTaskPointer(task);
}


TaskItem::TaskItem(QObject *parent, StartupPtr task)
    : AbstractGroupableItem(parent),
      d(new Private)
{
    d->startupTask = task;
    connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed(::TaskManager::TaskChanges)));
    connect(task.data(), SIGNAL(destroyed(QObject*)), this, SLOT(taskDestroyed())); //this item isn't useful anymore if the Task was closed
}

TaskItem::~TaskItem()
{
    emit destroyed(this);
    //kDebug();
    /*  if (parentGroup()) {
          parentGroup()->remove(this);
      }*/
    delete d;
}

void TaskItem::taskDestroyed()
{
    d->startupTask = 0;
    d->task.clear();
    // FIXME: due to a bug in Qt 4.x, the event loop reference count is incorrect
    // when going through x11EventFilter .. :/ so we have to singleShot the deleteLater
    QTimer::singleShot(0, this, SLOT(deleteLater()));
}

void TaskItem::setTaskPointer(TaskPtr task)
{
    const bool differentTask = d->task.data() != task.data();

    if (d->startupTask) {
        disconnect(d->startupTask.data(), 0, this, 0);
        d->startupTask = 0;
    } else if (differentTask) {
        // if we aren't moving from startup -> task and the task pointer is changing on us
        // let's clear the launcher url
        d->launcherUrl.clear();
    }

    if (differentTask) {
        if (d->task) {
            disconnect(d->task.data(), 0, this, 0);
        }

        d->task = task.data();

        if (task) {
            connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed(::TaskManager::TaskChanges)));
            connect(task.data(), SIGNAL(destroyed(QObject*)), this, SLOT(taskDestroyed()));
            emit gotTaskPointer();
        }
    }

    if (!d->task) {
        // FIXME: due to a bug in Qt 4.x, the event loop reference count is incorrect
        // when going through x11EventFilter .. :/ so we have to singleShot the deleteLater
        QTimer::singleShot(0, this, SLOT(deleteLater()));
    }
}

TaskPtr TaskItem::task() const
{
    if (!d->task) {
        return TaskPtr(0);
    }
    return TaskPtr(d->task.data());
}

StartupPtr TaskItem::startup() const
{
    /*
    if (d->startupTask.isNull()) {
        kDebug() << "pointer is Null";
    }
    */
    return d->startupTask;
}

bool TaskItem::isStartupItem() const
{
    return !d->startupTask.isNull();
}

WindowList TaskItem::winIds() const
{
    if (!d->task) {
        kDebug() << "no winId: probably startup task";
        return WindowList();
    }
    WindowList list;
    list << d->task.data()->window();
    return list;
}

QIcon TaskItem::icon() const
{
    if (d->task) {
        return d->task.data()->icon();
    }

    if (d->startupTask) {
        return d->startupTask->icon();
    }

    return QIcon();
}

QString TaskItem::name() const
{
    if (d->task) {
        return d->task.data()->visibleName();
    }

    if (d->startupTask) {
        return d->startupTask->text();
    }

    return QString();
}

QString TaskItem::taskName() const
{
    if (d->taskName.isEmpty()) {
        KUrl lUrl = launcherUrl();

        if (!lUrl.isEmpty() && lUrl.isLocalFile() && KDesktopFile::isDesktopFile(lUrl.toLocalFile())) {
            KDesktopFile f(lUrl.toLocalFile());

            if (f.tryExec()) {
                d->taskName = f.readName();
            }
        }
        if (d->taskName.isEmpty() && d->task) {
            d->taskName = d->task.data()->classClass().toLower();
        }
    }

    return d->taskName;
}

ItemType TaskItem::itemType() const
{
    return TaskItemType;
}

bool TaskItem::isGroupItem() const
{
    return false;
}

void TaskItem::setShaded(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setShaded(state);
}

void TaskItem::toggleShaded()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleShaded();
}

bool TaskItem::isShaded() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isShaded();
}

void TaskItem::toDesktop(int desk)
{
    if (!d->task) {
        return;
    }
    d->task.data()->toDesktop(desk);
}

bool TaskItem::isOnCurrentDesktop() const
{
    return d->task && d->task.data()->isOnCurrentDesktop();
}

bool TaskItem::isOnAllDesktops() const
{
    return d->task && d->task.data()->isOnAllDesktops();
}

int TaskItem::desktop() const
{
    if (!d->task) {
        return 0;
    }
    return d->task.data()->desktop();
}

void TaskItem::setMaximized(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setMaximized(state);
}

void TaskItem::toggleMaximized()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleMaximized();
}

bool TaskItem::isMaximized() const
{
    return d->task && d->task.data()->isMaximized();
}

void TaskItem::setMinimized(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setIconified(state);
}

void TaskItem::toggleMinimized()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleIconified();
}

bool TaskItem::isMinimized() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isMinimized();
}

void TaskItem::setFullScreen(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setFullScreen(state);
}

void TaskItem::toggleFullScreen()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleFullScreen();
}

bool TaskItem::isFullScreen() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isFullScreen();
}

void TaskItem::setKeptBelowOthers(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setKeptBelowOthers(state);
}

void TaskItem::toggleKeptBelowOthers()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleKeptBelowOthers();
}

bool TaskItem::isKeptBelowOthers() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isKeptBelowOthers();
}

void TaskItem::setAlwaysOnTop(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setAlwaysOnTop(state);
}

void TaskItem::toggleAlwaysOnTop()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleAlwaysOnTop();
}

bool TaskItem::isAlwaysOnTop() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isAlwaysOnTop();
}

bool TaskItem::isActionSupported(NET::Action action) const
{
    return d->task && d->task.data()->info().actionSupported(action);
}

void TaskItem::addMimeData(QMimeData *mimeData) const
{
    if (!d->task) {
        return;
    }

    d->task.data()->addMimeData(mimeData);
}

void TaskItem::setLauncherUrl(const KUrl &url)
{
    if (!d->launcherUrl.isEmpty()) {
        return;
    }
    d->launcherUrl = url;
    d->taskName = QString(); // Cause name to be re-generated...

    KConfig cfg("taskmanagerrulesrc");
    KConfigGroup grp(&cfg, "Mapping");
    grp.writeEntry(d->task.data()->classClass() + "::" + d->task.data()->className(), url.url());
    cfg.sync();
}

void TaskItem::setLauncherUrl(const AbstractGroupableItem *item)
{
    if (!d->launcherUrl.isEmpty() || !item) {
        return;
    }
    d->launcherUrl = item->launcherUrl();
    d->taskName = QString(); // Cause name to be re-generated...
}

static KService::List getServicesViaPid(int pid)
{
    // Attempt to find using commandline...
    KService::List services;
    KSysGuard::Processes procs;

    procs.updateOrAddProcess(pid);

    KSysGuard::Process *proc = procs.getProcess(pid);
    QString cmdline = proc ? proc->command.simplified() : QString(); // proc->command has a trailing space???

    if (!cmdline.isEmpty()) {
        int firstSpace = cmdline.indexOf(' ');

        services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Exec)").arg(cmdline));
        if (services.empty()) {
            // Could not find with complete commandline, so strip out path part...
            int slash = cmdline.lastIndexOf('/', firstSpace);
            if (slash > 0) {
                services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Exec)").arg(cmdline.mid(slash + 1)));
            }
        }

        if (services.empty() && firstSpace > 0) {
            // Could not find with arguments, so try without...
            cmdline = cmdline.left(firstSpace);
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Exec)").arg(cmdline));

            int slash = cmdline.lastIndexOf('/');
            if (slash > 0) {
                services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Exec)").arg(cmdline.mid(slash + 1)));
            }
        }
    }

    return services;
}

static KUrl getServiceLauncherUrl(int pid, const QString &type, const QStringList &cmdRemovals = QStringList())
{
    KSysGuard::Processes procs;

    procs.updateOrAddProcess(pid);

    KSysGuard::Process *proc = procs.getProcess(pid);
    QString cmdline = proc ? proc->command.simplified() : QString(); // proc->command has a trailing space???

    if (!cmdline.isEmpty()) {
        foreach (const QString & r, cmdRemovals) {
            cmdline.replace(r, "");
        }

        KService::List services = KServiceTypeTrader::self()->query(type, QString("exist Exec and ('%1' =~ Exec)").arg(cmdline));

        if (services.empty()) {
            // Could not find with complete commandline, so strip out path part...
            int slash = cmdline.lastIndexOf('/', cmdline.indexOf(' '));
            if (slash > 0) {
                services = KServiceTypeTrader::self()->query(type, QString("exist Exec and ('%1' =~ Exec)").arg(cmdline.mid(slash + 1)));
            }
        }

        if (!services.empty()) {
            QString path = services[0]->entryPath();

            if (!path.startsWith("/")) {
                QStringList dirs = KGlobal::dirs()->resourceDirs("services");
                foreach (const QString & d, dirs) {
                    if (QFile::exists(d + path)) {
                        path = d + path;
                        break;
                    }
                }
            }

            if (QFile::exists(path)) {
                return KUrl::fromPath(path);
            }
        }
    }

    return KUrl();
}

KUrl TaskItem::launcherUrl() const
{
    if (!d->task && !isStartupItem()) {
        return KUrl();
    }

    if (!d->launcherUrl.isEmpty() || d->checkedForLauncher) {
        return d->launcherUrl;
    }

    // Search for applications which are executable and case-insensitively match the windowclass of the task and
    // See http://techbase.kde.org/Development/Tutorials/Services/Traders#The_KTrader_Query_Language
    KService::List services;
    bool triedPid = false;

    // Set a flag so that we remeber that we have already checked for a launcher. This is becasue if we fail, then
    // we will keep on failing so the isEmpty()  check above is not enough.
    d->checkedForLauncher = true;

    if (d->task && !d->task.data()->classClass().isEmpty()) {

        // For KCModules, if we matchd on window class, etc, we would end up matching to kcmshell4 - but we are more than likely
        // interested in the actual control module. Thereffore we obtain this via the commandline. This commandline may contain
        // "kdeinit4:" or "[kdeinit]", so we remove these first.
        if ("Kcmshell4" == d->task.data()->classClass()) {
            d->launcherUrl = getServiceLauncherUrl(d->task.data()->pid(), "KCModule", QStringList() << "kdeinit4:" << "[kdeinit]");
            if (!d->launcherUrl.isEmpty()) {
                return d->launcherUrl;
            }
        }

        // Check to see if this wmClass matched a saved one...
        KConfig cfg("taskmanagerrulesrc");
        KConfigGroup grp(&cfg, "Mapping");
        KConfigGroup set(&cfg, "Settings");

        // Some apps have different laucnhers depending upon commandline...
        QStringList matchCommandLineFirst = set.readEntry("MatchCommandLineFirst", QStringList());
        if (matchCommandLineFirst.contains(d->task.data()->classClass())) {
            triedPid = true;
            services = getServicesViaPid(d->task.data()->pid());
        }

        // If the user has manualy set a mapping, respect this first...
        QString mapped(grp.readEntry(d->task.data()->classClass() + "::" + d->task.data()->className(), QString()));

        if (mapped.endsWith(".desktop")) {
            d->launcherUrl = mapped;
            return d->launcherUrl;
        }

        if (mapped.isEmpty()) {
            mapped = grp.readEntry(d->task.data()->classClass(), QString());

            if (mapped.endsWith(".desktop")) {
                d->launcherUrl = mapped;
                return d->launcherUrl;
            }
        }

        // Some apps, such as Wine, cannot use className to map to launcher name - as Wine itself is not a GUI app
        // So, Settings/ManualOnly lists window classes where the user will always ahve to manual set the launcher...
        QStringList manualOnly = set.readEntry("ManualOnly", QStringList());

        if (manualOnly.contains(d->task.data()->classClass())) {
            return d->launcherUrl;
        }

        if (!mapped.isEmpty()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ DesktopEntryName)").arg(mapped));
        }

        if (!mapped.isEmpty() && services.empty()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Name)").arg(mapped));
        }

        if (services.empty() && qobject_cast<GroupManager *>(parent())) {
            KUrl savedUrl = static_cast<GroupManager *>(parent())->launcherForWmClass(d->task.data()->classClass());
            if (savedUrl.isValid()) {
                d->launcherUrl = savedUrl;
                return d->launcherUrl;
            }
        }

        // To match other docks (docky, unity, etc.) attempt to match on DesktopEntryName first...
        if (services.empty()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ DesktopEntryName)").arg(d->task.data()->classClass()));
        }

        // Try StartupWMClass
        if (services.empty()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ StartupWMClass)").arg(d->task.data()->classClass()));
        }

        // Try 'Name' - unfortunately this can be translated, so has a good chance of failing! (As it does for KDE's own "System Settings" (even in English!!))
        if (services.empty()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Name)").arg(d->task.data()->classClass()));
        }

        // Ok, absolute *last* chance, try matching via pid (but only if we have not already tried this!)...
        if (services.empty() && !triedPid) {
            services = getServicesViaPid(d->task.data()->pid());
        }
    }

    if (services.empty() && isStartupItem()) {
        // Try to match via desktop filename...
        if (!startup()->desktopId().isNull() && startup()->desktopId().endsWith(".desktop")) {
            if (startup()->desktopId().startsWith("/")) {
                d->launcherUrl = KUrl::fromPath(startup()->desktopId());
                return d->launcherUrl;
            } else {
                QString desktopName = startup()->desktopId();

                if (desktopName.endsWith(".desktop")) {
                    desktopName = desktopName.mid(desktopName.length() - 8);
                }

                services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ DesktopEntryName)").arg(desktopName));
            }
        }

        // Try StartupWMClass
        if (services.empty() && !startup()->wmClass().isNull()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ StartupWMClass)").arg(startup()->wmClass()));
        }

        // Try via name...
        if (services.empty() && !startup()->text().isNull()) {
            services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and ('%1' =~ Name)").arg(startup()->text()));
        }
    }

    if (!services.empty()) {
        d->launcherUrl = KUrl::fromPath(services[0]->entryPath());
    }

    return d->launcherUrl;
}

void TaskItem::resetLauncherCheck()
{
    if (d->launcherUrl.isEmpty()) {
        d->checkedForLauncher = false;
    }
}

void TaskItem::close()
{
    if (!d->task) {
        return;
    }
    d->task.data()->close();
}

bool TaskItem::isActive() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isActive();
}

bool TaskItem::demandsAttention() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->demandsAttention();
}

} // TaskManager namespace

#include "taskitem.moc"
