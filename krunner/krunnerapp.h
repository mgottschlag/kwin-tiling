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

#include "restartingapplication.h"
#include "saverengine.h"
#include <kworkspace.h>
class KActionCollection;

class KRunnerApp : public RestartingApplication
{
    Q_OBJECT
public:
    KRunnerApp(Display *display,
                          Qt::HANDLE visual = 0,
                          Qt::HANDLE colormap = 0);
    void initializeShortcuts ();
    void logout( KWorkSpace::ShutdownConfirm confirm, KWorkSpace::ShutdownType sdtype );
    // The action collection of the active widget
    KActionCollection *actionCollection();

    virtual int newInstance();
    SaverEngine& screensaver() { return m_saver; }

signals:
    void showInterface();

public slots:
    /** Show minicli,. the KDE command line interface */
    void executeCommand();

    /** Show taskmanager (calls KSysGuard with --showprocesses option) */
    void showTaskManager();
    //void showWindowList();

    void switchUser();
    void logout();
    void logoutWithoutConfirmation();
    void haltWithoutConfirmation();
    void rebootWithoutConfirmation();

private:
    KActionCollection *m_actionCollection;
    SaverEngine m_saver;
};

#endif /* KRUNNERAPP_H */

