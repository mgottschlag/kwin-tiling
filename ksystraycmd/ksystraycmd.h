// -*- c++ -*-

#ifndef KSYSTRAYCMD_H
#define KSYSTRAYCMD_H

#include <qlabel.h>
#include <kwin.h>

class KShellProcess;
class KWinModule;

/**
 * Provides a system tray icon for a normal window.
 *
 * @author Richard Moore, rich@kde.org
 * @version $Id$
 */
class KSysTrayCmd : public QLabel
{
  Q_OBJECT
public:
  KSysTrayCmd();
  ~KSysTrayCmd();

  void setCommand( const QString &cmd ) { command = cmd; }
  void setPattern( const QString &regexp ) { window = regexp; }
  void setStartOnShow( bool enable ) { lazyStart = enable; }
  void setNoQuit( bool enable ) { noquit = enable; }
  void setQuitOnHide( bool enable ) { quitOnHide = enable; }
  void setDefaultTip( const QString &tip ) { tooltip = tip; }
  bool hasTargetWindow() const { return (win != 0); }
  bool hasRunningClient() const { return (client != 0); }
  const QString &errorMsg() const { return errStr; }

  bool start();

public slots:
  void refresh();

  void showWindow();
  void hideWindow();
  void toggleWindow() { if ( isVisible ) hideWindow(); else showWindow(); }

  void setTargetWindow( WId w );
  void execContextMenu( const QPoint &pos );

  void quit();

protected slots:
  void clientExited();

  void windowAdded(WId w);
  void windowChanged(WId w);

protected:
  bool startClient();
  void checkExistingWindows();
  void setTargetWindow( const KWin::Info &info );

  void mousePressEvent( QMouseEvent *e );
  void enterEvent( QEvent* );

private:
  QString command;
  QString window;
  QString tooltip;
  bool isVisible;
  bool lazyStart;
  bool noquit;
  bool quitOnHide;

  WId win;
  KShellProcess *client;
  KWinModule *kwinmodule;
  QString errStr;
};

#endif // KSYSTRAYCMD_H
