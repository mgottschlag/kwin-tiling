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

#include <kworkspace.h>

#include "restartingapplication.h"
#include "saverengine.h"

class KActionCollection;
class KDialog;
class Interface;
class StartupId;

class KRunnerApp : public RestartingApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner.App")

public:
/*    KRunnerApp(Display *display,
                          Qt::HANDLE visual = 0,
                          Qt::HANDLE colormap = 0);*/
    KRunnerApp();
    ~KRunnerApp();

    void logout( KWorkSpace::ShutdownConfirm confirm, KWorkSpace::ShutdownType sdtype );
    // The action collection of the active widget
    KActionCollection *actionCollection();

    virtual int newInstance();
    SaverEngine& screensaver() { return m_saver; }


    //UGLY
    static bool s_haveCompositeManager;

public slots:
    //void showWindowList();

    void logout();
    void logoutWithoutConfirmation();
    void haltWithoutConfirmation();
    void rebootWithoutConfirmation();

    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml -s krunnerapp.h -o org.kde.krunner.App.xml
    Q_SCRIPTABLE void initializeStartupNotification();
    
    /** Show taskmanager */
    Q_SCRIPTABLE void showTaskManager();

private slots:
    /**
     * Called when the task dialog emits its finished() signal
     */
    void taskDialogFinished();

private:
    void initialize();

    KActionCollection *m_actionCollection;
    SaverEngine m_saver;
    Interface* m_interface;
    KDialog* m_tasks;
    StartupId* m_startupId;
};

#endif /* KRUNNERAPP_H */

