/* This file is part of the KDE project
   Copyright (C) 2007       Troy Unrau <troy.unrau@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//#include <krootwm.h>
#include <kaction.h>
#include <kglobalaccel.h>
#include <kauthorized.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <klocale.h>
#include <kprocess.h>
#include <KActionCollection>
#include <QObject>
#include "../lib/kworkspace.h"

//class KMessageBox;

class KRunnerBindings : public QWidget {
  Q_OBJECT

public:

  void setBindings ();
  void logout();
  void logout( KWorkSpace::ShutdownConfirm confirm, KWorkSpace::ShutdownType sdtype );
// The action collection of the active widget
  KActionCollection *actionCollection();

private Q_SLOTS:
  /** Show minicli,. the KDE command line interface */
  void slotExecuteCommand();

  /** Show taskmanager (calls KSysGuard with --showprocesses option) */
  void slotShowTaskManager();

  void slotShowWindowList();

  void slotSwitchUser();

  void slotLogout();
  void slotLogoutNoCnf();
  void slotHaltNoCnf();
  void slotRebootNoCnf();

private:

  KActionCollection *m_actionCollection;

  };

