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

#include <QWidget>
#include <QStringList>

#include <dbus/qdbus.h>
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
class KDesktop : public QWidget
{
  Q_OBJECT

public:

  enum WheelDirection { Forward = 0, Reverse };

  KDesktop(bool x_root_hack, bool wait_for_kded );
  ~KDesktop();

  // Implementation of the DBus interface

  /**
   * Re-arrange the desktop icons.
   */
  void rearrangeIcons();
  /**
   * Lineup the desktop icons.
   */
  void lineupIcons();
  /**
   * Select all icons
   */
  void selectAll();
  /**
   * Unselect all icons
   */
  void unselectAll();
  /**
   * Refresh all icons
   */
  void refreshIcons();
  /**
   * @return the urls of selected icons
   */
  QStringList selectedURLs();
  /**
   * Re-read KDesktop's configuration
   */
  void configure();
  /**
   * Display the "Run Command" dialog (minicli) and prefill with @p content
   */
  void popupExecuteCommand(const QString& content);
  /**
   * Full refresh
   */
  void refresh();
  /**
   * Bye bye
   */
  void logout();
  /**
   * Clears the command history and completion items
   */
  void clearCommandHistory();
  /**
   * @internal
   */
  void runAutoStart();

  /**
   * Should be called by any application that wants to tell KDesktop
   * to switch desktops e.g.  the minipager applet on kicker.
   */
  void switchDesktops( int delta );

  /**
   * Returns whether KDesktop uses a virtual root.
   */
  bool isVRoot() { return set_vroot; }
  /**
   * Set whether KDesktop should use a virtual root.
   */
  void setVRoot( bool enable );

  /**
   * Returns whether icons are enabled on the desktop
   */
  bool isIconsEnabled() { return m_bDesktopEnabled; }
  /**
   * Enable/disable icons on the desktop.
   */
 void setIconsEnabled( bool enable );

  /**
   * Get the dbus object path for the background interface
   */
  QDBusObjectPath background();
  /**
   * Get the dbus object path for the screensaver interface
   */
  QDBusObjectPath screenSaver();

  // End of DBus-exported methods


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
