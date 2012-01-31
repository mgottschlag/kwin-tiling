/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KRUNNERAPP_H
#define KRUNNERAPP_H

#include <kuniqueapplication.h>

class KActionCollection;
class KDialog;

namespace Plasma
{
    class RunnerManager;
}

class KRunnerDialog;
class KSystemActivityDialog;
class StartupId;

class KRunnerApp : public KUniqueApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner.App")

public:
    static KRunnerApp* self();
    ~KRunnerApp();

    // The action collection of the active widget
    KActionCollection* actionCollection();

    virtual int newInstance();

public Q_SLOTS:
    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml -m krunnerapp.h -o dbus/org.kde.krunner.App.xml
    Q_SCRIPTABLE void initializeStartupNotification();

    /** Show taskmanager */
    Q_SCRIPTABLE void showTaskManager();
    /** Show taskmanager, filtering by the given string */
    Q_SCRIPTABLE void showTaskManagerWithFilter(const QString &filterText);

    /** Display the interface */
    Q_SCRIPTABLE void display();

    /** Enter single runner query mode **/
    Q_SCRIPTABLE void displaySingleRunner(const QString& runnerName);

    /** Display the interface, using clipboard contents */
    Q_SCRIPTABLE void displayWithClipboardContents();

    /** Display the interface */
    Q_SCRIPTABLE void query(const QString& term);

    /** Enter single runner query mode **/
    Q_SCRIPTABLE void querySingleRunner(const QString& runnerName, const QString &term);

    /** Switch user */
    Q_SCRIPTABLE void switchUser();

    /** Clear the search history */
    Q_SCRIPTABLE void clearHistory();

    Q_SCRIPTABLE QStringList singleModeAdvertisedRunnerIds() const;

private slots:
    /**
     * Called when the task dialog emits its finished() signal
     */
    void taskDialogFinished();
    void reloadConfig();
    void cleanUp();
    void displayOrHide();
    void singleRunnerModeActionTriggered();

private:
    KRunnerApp();
    void initialize();

    Plasma::RunnerManager *m_runnerManager;
    KActionCollection *m_actionCollection;
    KRunnerDialog *m_interface;
    KSystemActivityDialog *m_tasks;
    StartupId *m_startupId;
    bool m_firstTime;
};

#endif /* KRUNNERAPP_H */

