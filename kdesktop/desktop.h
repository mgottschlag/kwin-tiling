/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __desktop_h__
#define __desktop_h__

#include "KDesktopIface.h"

#include <QWidget>
#include <qstringlist.h>

#include <kworkspace.h>	// for logout parameters

class KUrl;
class QCloseEvent;
class QDropEvent;
class QPopupMenu;
class KGlobalAccel;
class KWinModule;
class KBackgroundManager;
class QTimer;
class StartupId;
class KDIconView;
class Minicli;
class KActionCollection;

class KRootWidget : public QObject
{
    Q_OBJECT
public:
   KRootWidget();
   bool eventFilter( QObject *, QEvent * e );
Q_SIGNALS:
   void wheelRolled( int delta );
   void colorDropEvent( QDropEvent* e );
   void imageDropEvent( QDropEvent* e );
   void newWallpaper( const KUrl& url );
};

/**
 * KDesktop is the toplevel widget that is the desktop.
 * It handles the background, the screensaver and all the rest of the global stuff.
 * The icon view is a child widget of KDesktop.
 */
class KDesktop : public QWidget, virtual public KDesktopIface
{
  Q_OBJECT

public:

  enum WheelDirection { Forward = 0, Reverse };

  KDesktop(bool x_root_hack, bool wait_for_kded );
  ~KDesktop();

  // Implementation of the DCOP interface
  virtual void rearrangeIcons();
  virtual void lineupIcons();
  virtual void selectAll();
  virtual void unselectAll();
  virtual void refreshIcons();
  virtual QStringList selectedURLs();

  virtual void configure();
  virtual void popupExecuteCommand();
  virtual void popupExecuteCommand(const QString& content);
  virtual void refresh();
  virtual void logout();
  virtual void clearCommandHistory();
  virtual void runAutoStart();

  virtual void switchDesktops( int delta );

  void logout( KWorkSpace::ShutdownConfirm confirm, KWorkSpace::ShutdownType sdtype );

  KWinModule* kwinModule() const { return m_pKwinmodule; }

  // The action collection of the active widget
  KActionCollection *actionCollection();

  // The URL (for the File/New menu)
  KUrl url() const;

  // ## hack ##
  KDIconView *iconView() const { return m_pIconView; }

private Q_SLOTS:

  void workAreaChanged();

  /** Background is ready. */
  void backgroundInitDone();

  /** Activate the desktop. */
  void slotStart();

  /** Activate crash recovery. */
  void slotUpAndRunning();

  /** Reconfigures */
  void slotConfigure();

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

  /** Connected to KSycoca */
  void slotDatabaseChanged();

  void slotShutdown();
  void slotSettingsChanged(int);
  void slotIconChanged(int);

  /** set the vroot atom for e.g. xsnow */
  void slotSetVRoot();

  /** Connected to KDIconView */
  void handleImageDropEvent( QDropEvent * );
  void handleColorDropEvent( QDropEvent * );
  void slotNewWallpaper(const KUrl &url);

  void updateWorkArea();

  /** Connected to KDIconView and KRootWidget  */
  void slotSwitchDesktops(int delta);

protected:
  void initConfig();
  void initRoot();

  virtual void closeEvent(QCloseEvent *e);

  virtual bool isVRoot() { return set_vroot; }
  virtual void setVRoot( bool enable );

  virtual bool isIconsEnabled() { return m_bDesktopEnabled; }
  virtual void setIconsEnabled( bool enable );
  virtual bool event ( QEvent * e );

private Q_SLOTS:
  void desktopResized();

private:

  KActionCollection *m_actionCollection;

  KWinModule* m_pKwinmodule;

  KBackgroundManager* bgMgr;

  KDIconView *m_pIconView;
  KRootWidget *m_pRootWidget;

  QTimer *updateWorkAreaTimer;

  Minicli *m_miniCli;

  StartupId* startup_id;
  bool set_vroot;

  /** Set to true until start() has been called */
  bool m_bInit;

  /** Wait for kded to finish building database? */
  bool m_bWaitForKded;

  /** Desktop enabled / disabled **/
  bool m_bDesktopEnabled;

  /** Whether or not to switch desktops when mouse wheel is rolled */
  bool m_bWheelSwitchesWorkspace;

  /** Default mouse wheel direction (Fwd means mwheel up switches to
      lower desktop)
  */
  static const WheelDirection m_eDefaultWheelDirection = Forward;

  /** Mouse wheel/desktop switching direction */
  static WheelDirection m_eWheelDirection;

  /** Possible values for "kdesktoprc"->"Mouse Buttons"->"WheelDirection" */
  static const char* m_wheelDirectionStrings[2];
};

#endif
