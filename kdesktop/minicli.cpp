/* This file is part of the KDE project

   Copyright (C) 1999-2002,2003 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2000 Malte Starostik <starosti@zedat.fu-berlin.de>
   Copyright (C) 2003 Sven Leiber <s.leiber@web.de>

   Kdesu integration:
   Copyright (C) 2000 Geert Jansen <jansen@kde.org>

   Original authors:
   Copyright (C) 1997 Matthias Ettrich <ettrich@kde.org>
   Copyright (C) 1997 Torben Weis [ Added command completion ]
   Copyright (C) 1999 Preston Brown <pbrown@kde.org>

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

#include <pwd.h>
#include <string.h>
#include <errno.h>

#include <Q3VBox>
#include <qlabel.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qslider.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kpassdlg.h>
#include <krun.h>
#include <kwin.h>
#include <kdesu/su.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <kstdguiitem.h>
#include <kmimetype.h>
#include <kurifilter.h>
#include <kcompletionbox.h>
#include <kworkspace.h>

#include "minicli.moc"
#include "minicli_ui.h"
#include "kdesktopsettings.h"

#include <QDesktopWidget>
#include <QKeyEvent>
#include <QPainter>
#include <QTextStream>
#include <Q3StyleSheet>
#include <kauthorized.h>

#define KDESU_ERR strerror(errno)

Minicli::Minicli( QWidget *parent, const char *name)
        :KDialog( parent, name ),
         m_autoCheckedRunInTerm(false)
{
  setPlainCaption( i18n("Run Command") );
  KWin::setIcons( winId(), DesktopIcon("run"), SmallIcon("run") );

  QVBoxLayout* mainLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  m_dlg = new MinicliDlgUI (this);
  mainLayout->addWidget(m_dlg);

  m_dlg->lbRunIcon->setPixmap(DesktopIcon("kmenu"));
  m_dlg->lbComment->setAlignment( Qt::TextWordWrap );

  m_dlg->cbCommand->setDuplicatesEnabled( false );
  m_dlg->cbCommand->setTrapReturnKey( true );

  // Options button...
  m_dlg->pbOptions->setGuiItem (KGuiItem( i18n("&Options >>"), "configure" ));

  // Run button...
  m_dlg->pbRun->setGuiItem (KGuiItem( i18n("&Run"), "run" ));

  // Cancel button...
  m_dlg->pbCancel->setGuiItem ( KStdGuiItem::cancel() );

  if (!KAuthorized::authorizeKAction("shell_access"))
    m_dlg->pbOptions->hide();

  m_dlg->pbRun->setEnabled(!m_dlg->cbCommand->currentText().isEmpty());
  m_dlg->pbRun->setDefault( true );

  // Do not show the advanced group box on startup...
  m_dlg->gbAdvanced->hide();

  // URI Filter meta object...
  m_filterData = new KURIFilterData();

  // Create a timer object...
  m_parseTimer = new QTimer(this);

  m_FocusWidget = 0;

  m_prevCached = false;
  m_iPriority = 50;
  m_iScheduler = StubProcess::SchedNormal;

  m_dlg->leUsername->setText("root");

  setMaxCommandBoxWidth();

  // Main widget buttons...
  connect( m_dlg->pbRun, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( m_dlg->pbCancel, SIGNAL(clicked()), this, SLOT(reject()) );
  connect( m_dlg->pbOptions, SIGNAL(clicked()), SLOT(slotAdvanced()) );
  connect( m_parseTimer, SIGNAL(timeout()), SLOT(slotParseTimer()) );

  connect( m_dlg->cbCommand, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotCmdChanged(const QString&) ) );

  connect( m_dlg->cbCommand, SIGNAL( returnPressed() ),
           m_dlg->pbRun, SLOT( animateClick() ) );

  // Advanced group box...
  connect(m_dlg->cbPriority, SIGNAL(toggled(bool)), SLOT(slotChangeScheduler(bool)));
  connect(m_dlg->slPriority, SIGNAL(valueChanged(int)), SLOT(slotPriority(int)));
  connect(m_dlg->cbRealtime, SIGNAL(toggled(bool)), SLOT(slotRealtime(bool)));
  connect(m_dlg->cbRunAsOther, SIGNAL(toggled(bool)), SLOT(slotChangeUid(bool)));
  connect(m_dlg->leUsername, SIGNAL(lostFocus()), SLOT(updateAuthLabel()));
  connect(m_dlg->cbRunInTerminal, SIGNAL(toggled(bool)), SLOT(slotTerminal(bool)));

  m_dlg->slPriority->setValue(50);

  loadConfig();
}

Minicli::~Minicli()
{
  delete m_filterData;
}

void Minicli::setCommand(const QString& command)
{
  m_dlg->cbCommand->lineEdit()->setText(command);
  m_dlg->cbCommand->lineEdit()->deselect();
  int firstSpace = command.find(' ');
  if (firstSpace > 0) {
    m_dlg->cbCommand->lineEdit()->setSelection(firstSpace+1, command.length());
  }
}

void Minicli::setMaxCommandBoxWidth()
{
  int maxWidth = qApp->desktop()->screenGeometry(this).width();
  if (maxWidth < 603)
  {
    // a sensible max for smaller screens
    maxWidth = (maxWidth > 240) ? 240 : maxWidth;
  }
  else
  {
    maxWidth = maxWidth * 2 / 5;
  }

  m_dlg->cbCommand->setMaximumWidth(maxWidth);
}

void Minicli::show()
{
  setMaxCommandBoxWidth();
  KWin::setState( winId(), NET::StaysOnTop );
  setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
  adjustSize();
  setFixedSize(size());
  KDialog::show();
  adjustSize();
  setFixedSize(size());
}

void Minicli::loadConfig()
{
  QStringList histList = KDesktopSettings::history();
  int maxHistory = KDesktopSettings::historyLength();
  m_terminalAppList = KDesktopSettings::terminalApps();

  if (m_terminalAppList.isEmpty())
    m_terminalAppList << "ls"; // Default

  bool block = m_dlg->cbCommand->signalsBlocked();
  m_dlg->cbCommand->blockSignals( true );
  m_dlg->cbCommand->setMaxCount( maxHistory );
  m_dlg->cbCommand->setHistoryItems( histList );
  m_dlg->cbCommand->blockSignals( block );

  QStringList compList = KDesktopSettings::completionItems();
  if( compList.isEmpty() )
    m_dlg->cbCommand->completionObject()->setItems( histList );
  else
    m_dlg->cbCommand->completionObject()->setItems( compList );

  int mode = KDesktopSettings::completionMode();
  m_dlg->cbCommand->setCompletionMode( (KGlobalSettings::Completion) mode );

  KCompletionBox* box = m_dlg->cbCommand->completionBox();
  if (box)
    box->setActivateOnSelect( false );

  m_finalFilters = KURIFilter::self()->pluginNames();
  m_finalFilters.remove("kuriikwsfilter");

  m_middleFilters = m_finalFilters;
  m_middleFilters.remove("localdomainurifilter");

  // Provide username completions. Use saner and configurable maximum values.
  int maxEntries = KDesktopSettings::maxUsernameCompletions();
  QStringList users;

  struct passwd *pw;
  setpwent();
  for (int count=0; ((pw = getpwent()) != 0L) && (count < maxEntries); count++)
    users << QString::fromLocal8Bit(pw->pw_name);
  endpwent();

  KCompletion *completion = new KCompletion;
  completion->setOrder(KCompletion::Sorted);
  completion->insertItems (users);

  m_dlg->leUsername->setCompletionObject(completion, true);
  m_dlg->leUsername->setCompletionMode(KGlobalSettings::completionMode());
  m_dlg->leUsername->setAutoDeleteCompletionObject( true );

}

void Minicli::saveConfig()
{
  KDesktopSettings::setHistory( m_dlg->cbCommand->historyItems() );
  KDesktopSettings::setTerminalApps( m_terminalAppList );
  KDesktopSettings::setCompletionItems( m_dlg->cbCommand->completionObject()->items() );
  KDesktopSettings::setCompletionMode( m_dlg->cbCommand->completionMode() );
  KDesktopSettings::writeConfig();
}

void Minicli::clearHistory()
{
  m_dlg->cbCommand->clearHistory();
  saveConfig();
}

void Minicli::accept()
{
  QString cmd = m_dlg->cbCommand->currentText().trimmed();
  if (!cmd.isEmpty() && (cmd[0].isNumber() || (cmd[0] == '(')) &&
      (QRegExp("[a-zA-Z\\]\\[]").search(cmd) == -1))
  {
     QString result = calculate(cmd);
     if (!result.isEmpty())
        m_dlg->cbCommand->setCurrentText(result);
     return;
  }

  bool logout = (cmd == "logout");
  if( !logout && runCommand() == 1 )
     return;

  m_dlg->cbCommand->addToHistory( m_dlg->cbCommand->currentText().trimmed() );
  reset();
  saveConfig();
  QDialog::accept();

  if ( logout )
  {
     KWorkSpace::propagateSessionManager();
     KWorkSpace::requestShutDown();
  }
}

void Minicli::reject()
{
  reset();
  QDialog::reject();
}

void Minicli::reset()
{
  if( m_dlg->gbAdvanced->isShown() )
    slotAdvanced();

  bool block = m_dlg->cbCommand->signalsBlocked();
  m_dlg->cbCommand->blockSignals( true );
  m_dlg->cbCommand->clearEdit();
  m_dlg->cbCommand->setFocus();
  m_dlg->cbCommand->reset();
  m_dlg->cbCommand->blockSignals( block );
  m_dlg->pbRun->setEnabled( false );

  m_iPriority = 50;
  m_iScheduler = StubProcess::SchedNormal;

  m_dlg->cbRunInTerminal->setChecked(false);
  m_dlg->cbRunAsOther->setChecked(false);

  m_dlg->leUsername->setText("root");

  m_dlg->cbPriority->setChecked(false);

  m_dlg->slPriority->setValue(m_iPriority);

  m_dlg->cbRealtime->setChecked( m_iScheduler == StubProcess::SchedRealtime );
  m_dlg->lePassword->erase();

  m_FocusWidget = 0;
  m_iconName = QString::null;
  m_prevIconName = QString::null;

  m_prevCached = false;
  updateAuthLabel();
  setIcon();
}

void Minicli::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    e->accept();
    m_dlg->pbCancel->animateClick();
    return;
  }

  QDialog::keyPressEvent( e );
}

QString Minicli::terminalCommand (const QString& cmd, const QString& args)
{
  QString terminal = KDesktopSettings::terminalApplication().trimmed();
  if (terminal.endsWith("konsole"))
    terminal += " --noclose";

  if( args.isEmpty() )
    terminal += QString(" -e /bin/sh -c \"%1\"").arg(cmd);
  else
    terminal += QString(" -e /bin/sh -c \"%1 %2\"").arg(cmd).arg(args);

  if (!m_terminalAppList.contains(cmd))
    m_terminalAppList << cmd;

  return terminal;
}

int Minicli::runCommand()
{
  m_parseTimer->stop();

  // Make sure we have an updated data
  parseLine( true );

  // Ignore empty commands...
  if ( m_dlg->cbCommand->currentText().isEmpty() )
    return 1;

  QString cmd;
  KURL uri = m_filterData->uri();
  if ( uri.isLocalFile() && !uri.hasRef() && uri.query().isEmpty() )
    cmd = uri.path();
  else
    cmd = uri.url();

  // Determine whether the application should be run through
  // the command line (terminal) interface...
  bool useTerminal = m_dlg->cbRunInTerminal->isChecked();

  kdDebug (1207) << "Use terminal ? " << useTerminal << endl;

  if (!KAuthorized::authorizeKAction("shell_access"))
    useTerminal = false;

  if( needsKDEsu() )
  {
    QByteArray user;
    struct passwd *pw;

    if (m_dlg->cbRunAsOther)
    {
      pw = getpwnam(m_dlg->leUsername->text().toLocal8Bit());
      if (pw == 0L)
      {
          KMessageBox::sorry( this, i18n("<qt>The user <b>%1</b> "
          "does not exist on this system.</qt>").arg(m_dlg->leUsername->text()));
        return 1;
      }
    }
    else
    {
      pw = getpwuid(getuid());
      if (pw == 0L)
      {
          KMessageBox::error( this, i18n("You do not exist.\n"));
        return 1;
      }
    }

    user = pw->pw_name;

    {
      // Scoped. we want this object to go away before the fork
      // (maybe not necessary, but can't hurt) according to the cvs log,
      // creating the SuProcess object in the parent and using it in the
      // child creates crashes, but we need to check password before the
      // fork in order to not get in trouble with X for the messagebox

      SuProcess proc_checkpwd;
      proc_checkpwd.setUser( user );

      if (m_dlg->cbPriority->isChecked())
      {
        proc_checkpwd.setPriority(m_iPriority);
        proc_checkpwd.setScheduler(m_iScheduler);
      }

      if (proc_checkpwd.checkInstall(m_dlg->lePassword->password()) != 0)
      {
          KMessageBox::sorry(this, i18n("Incorrect password; please try again."));
        return 1;
      }
    }

    QApplication::flushX();

    int pid = fork();

    if (pid < 0)
    {
      kdError(1207) << "fork(): " << KDESU_ERR << "\n";
      return -1;
    }

    if (pid > 0)
      return 0;

    // From here on, this is child...

    SuProcess proc;
    proc.setUser(user);

    if (m_dlg->cbPriority->isChecked())
    {
      proc.setPriority(m_iPriority);
      proc.setScheduler(m_iScheduler);
    }

    QByteArray command;

    if (useTerminal)
      command = terminalCommand( cmd, m_filterData->argsAndOptions() ).toLocal8Bit();
    else
    {
      command = cmd.toLocal8Bit();
      if( m_filterData->hasArgsAndOptions() )
        command += m_filterData->argsAndOptions().toLocal8Bit();
    }

    proc.setCommand(command);

    // Block SIGCHLD because SuProcess::exec() uses waitpid()
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sset, 0L);
    proc.setTerminal(true);
    proc.setErase(true);
    _exit(proc.exec(m_dlg->lePassword->password()));
  }
  else
  {
    QString exec;

    // yes, this is a hack, but there is no way of doing it
    // through SuProcess without providing the user password
    if (m_iPriority < 50)
    {
      // from kdesu_stub.c
      int val = 20 - (int) (((double) m_iPriority) * 40 / 100 + 0.5);
      cmd = "nice -n " + QString::number( val ) + " " + cmd;
    }

    if (useTerminal)
    {
      cmd = terminalCommand( cmd, m_filterData->argsAndOptions() );
      kdDebug(1207) << "Terminal command: " << cmd << endl;
    }
    else
    {
      switch( m_filterData->uriType() )
      {
        case KURIFilterData::LOCAL_FILE:
        case KURIFilterData::LOCAL_DIR:
        case KURIFilterData::NET_PROTOCOL:
        case KURIFilterData::HELP:
        {
          // No need for kfmclient, KRun does it all (David)
          (void) new KRun( m_filterData->uri(), 0 );
          return 0;
        }
        case KURIFilterData::EXECUTABLE:
        {
          if( !m_filterData->hasArgsAndOptions() )
          {
            // Look for desktop file
            KService::Ptr service = KService::serviceByDesktopName(cmd);
            if (service && service->isValid() && service->type() == "Application")
            {
              notifyServiceStarted(service);
              KRun::run(*service, KURL::List());
              return 0;
            }
          }
        }
        case KURIFilterData::SHELL:
        {
          if (KAuthorized::authorizeKAction("shell_access"))
          {
            exec = cmd;

            if( m_filterData->hasArgsAndOptions() )
              cmd += m_filterData->argsAndOptions();

            break;
          }
          else
          {
            KMessageBox::sorry( this, i18n("<center><b>%1</b></center>\n"
                                      "You do not have permission to execute "
                                      "this command.")
                                      .arg( Q3StyleSheet::convertFromPlainText(cmd) ));
            return 1;
          }
        }
        case KURIFilterData::UNKNOWN:
        case KURIFilterData::ERROR:
        default:
        {
          // Look for desktop file
          KService::Ptr service = KService::serviceByDesktopName(cmd);
          if (service && service->isValid() && service->type() == "Application")
          {
            notifyServiceStarted(service);
            KRun::run(*service, KURL::List());
            return 0;
          }

          service = KService::serviceByName(cmd);
          if (service && service->isValid() && service->type() == "Application")
          {
            notifyServiceStarted(service);
            KRun::run(*service, KURL::List());
            return 0;
          }

          KMessageBox::sorry( this, i18n("<center><b>%1</b></center>\n"
                                    "Could not run the specified command.")
                                    .arg( Q3StyleSheet::convertFromPlainText(cmd) ));
          return 1;
        }
      }
    }

    if ( KRun::runCommand( cmd, exec, m_iconName ) )
      return 0;
    else
    {
      KMessageBox::sorry( this, i18n("<center><b>%1</b></center>\n"
                                "The specified command does not exist.").arg(cmd) );
      return 1; // Let the user try again...
    }
  }
}

void Minicli::notifyServiceStarted(KService::Ptr service)
{
    // Inform other applications (like the quickstarter applet)
    // that an application was started
    QByteArray params;
    QDataStream stream(&params, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);
    stream << "minicli" << service->storageId();
    kdDebug() << "minicli appLauncher dcop signal: " << service->storageId() << endl;
    KApplication::kApplication()->dcopClient()->emitDCOPSignal("appLauncher",
        "serviceStartedByStorageId(QString,QString)", params);
}

void Minicli::slotCmdChanged(const QString& text)
{
  bool isEmpty = text.isEmpty();
  m_dlg->pbRun->setEnabled( !isEmpty );

  if( isEmpty )
  {
    // Reset values to default
    m_filterData->setData(KURL());

    // Empty String is certainly no terminal application
    slotTerminal(false);

    // Reset the icon if needed...
    const QPixmap pixmap = DesktopIcon("kmenu");

    if ( pixmap.serialNumber() != m_dlg->lbRunIcon->pixmap()->serialNumber())
      m_dlg->lbRunIcon->setPixmap(pixmap);

    return;
  }

  m_parseTimer->start(250, true);
}

void Minicli::slotAdvanced()
{
  if (m_dlg->gbAdvanced->isHidden())
  {
    m_dlg->gbAdvanced->show();
    m_dlg->pbOptions->setText(i18n("&Options <<"));

    // Set the focus back to the widget that had it to begin with, i.e.
    // do not put the focus on the "Options" button.
    m_FocusWidget = focusWidget();

    if( m_FocusWidget )
      m_FocusWidget->setFocus();
  }
  else
  {
    m_dlg->gbAdvanced->hide();
    m_dlg->pbOptions->setText(i18n("&Options >>"));

    if( m_FocusWidget && m_FocusWidget->parent() != m_dlg->gbAdvanced )
      m_FocusWidget->setFocus();
  }
  adjustSize();
}

void Minicli::slotParseTimer()
{
  //kdDebug (1207) << "Minicli::slotParseTimer: Timed out..." << endl;
  parseLine( false );
}

void Minicli::parseLine( bool final )
{
  QString cmd = m_dlg->cbCommand->currentText().trimmed();
  m_filterData->setData( cmd );

  if( final )
    KURIFilter::self()->filterURI( *(m_filterData), m_finalFilters );
  else
    KURIFilter::self()->filterURI( *(m_filterData), m_middleFilters );

  bool isTerminalApp = ((m_filterData->uriType() == KURIFilterData::EXECUTABLE) &&
                        m_terminalAppList.contains(m_filterData->uri().url()));

  if( !isTerminalApp )
  {
    m_iconName = m_filterData->iconName();
    setIcon();
  }

  if ( isTerminalApp && final && !m_dlg->cbRunInTerminal->isChecked() )
  {
    m_terminalAppList.remove( m_filterData->uri().url() );
    isTerminalApp = false;
  }
  else
  {
    bool wasAutoChecked = m_autoCheckedRunInTerm;
    bool willBeAutoChecked = isTerminalApp && !m_dlg->cbRunInTerminal->isChecked();
    slotTerminal(isTerminalApp || (m_dlg->cbRunInTerminal->isChecked() && !m_autoCheckedRunInTerm));
    if (!wasAutoChecked && willBeAutoChecked)
        m_autoCheckedRunInTerm = true;
  }

  kdDebug (1207) << "Command: " << m_filterData->uri().url() << endl;
  kdDebug (1207) << "Arguments: " << m_filterData->argsAndOptions() << endl;
}

void Minicli::setIcon ()
{
  if( m_iconName.isEmpty() || m_iconName == "unknown" || m_iconName == "kde" )
    m_iconName = QLatin1String("kmenu");

  QPixmap icon = DesktopIcon( m_iconName );

  if ( m_iconName == "www" )
  {
    // Not using KIconEffect::overlay as that requires the same size
    // for the icon and the overlay, also the overlay definately doesn't
    // have a more that one-bit alpha channel here
#ifdef __GNUC__
#warning "Yet another overlay thingie!"    
#endif
    QPixmap overlay( locate ( "icon", KMimeType::favIconForURL( m_filterData->uri() ) + ".png" ) );
    if ( !overlay.isNull() )
    {
      int x = icon.width() - overlay.width();
      int y = icon.height() - overlay.height();
      if ( !icon.mask().isNull() )
      {
        QBitmap mask = icon.mask();
        if (!mask.isNull())
        {
            //Combine the two masks.
            QPainter p(&mask);
            QRegion clipMask(overlay.mask());
            clipMask.translate(x, y);
            p.setClipRegion(clipMask);
            p.fillRect(x, y, overlay.width(), overlay.height(), Qt::color1);
            p.end();
            icon.setMask(mask);
        }
      }
      bitBlt( &icon, x, y, &overlay );
    }
  }

  m_dlg->lbRunIcon->setPixmap( icon );
}

void Minicli::updateAuthLabel()
{
  if (m_dlg->cbPriority->isChecked() && (m_iPriority > 50) ||
      (m_iScheduler != StubProcess::SchedNormal))
  {
    if (!m_prevCached && !m_dlg->leUsername->text().isEmpty())
    {
      //kdDebug(1207) << k_funcinfo << "Caching: user=" << m_dlg->leUsername->text() <<
      //  ", checked=" << m_dlg->cbRunAsOther->isChecked() << endl;

      m_prevUser = m_dlg->leUsername->text();
      m_prevPass = m_dlg->lePassword->text();
      m_prevChecked = m_dlg->cbRunAsOther->isChecked();
      m_prevCached = true;
    }
    if (m_dlg->leUsername->text() != QLatin1String("root"))
      m_dlg->lePassword->setText(QString::null);
    m_dlg->leUsername->setText(QLatin1String("root"));
    m_dlg->cbRunAsOther->setChecked(true);
    m_dlg->cbRunAsOther->setEnabled(false);
    m_dlg->leUsername->setEnabled(false);
    m_dlg->lbUsername->setEnabled(true);
    m_dlg->lePassword->setEnabled(true);
    m_dlg->lbPassword->setEnabled(true);
  }
  else if (m_dlg->cbRunAsOther->isEnabled() &&
    m_dlg->cbRunAsOther->isChecked() && !m_dlg->leUsername->text().isEmpty())
  {
    m_dlg->lePassword->setEnabled(true);
    m_dlg->lbPassword->setEnabled(true);
  }
  else
  {
    if (m_prevCached)
    {
      m_dlg->leUsername->setText(m_prevUser);
      m_dlg->lePassword->setText(m_prevPass);
      m_dlg->cbRunAsOther->setChecked(m_prevChecked);
      m_dlg->leUsername->setEnabled(m_prevChecked);
      m_dlg->lbUsername->setEnabled(m_prevChecked);
    }
    else
    {
      m_dlg->cbRunAsOther->setChecked(false);
      m_dlg->leUsername->setEnabled(false);
      m_dlg->lbUsername->setEnabled(false);
    }
    m_dlg->cbRunAsOther->setEnabled(true);
    m_dlg->lePassword->setEnabled(false);
    m_dlg->lbPassword->setEnabled(false);
    m_prevCached = false;
  }
}

void Minicli::slotTerminal(bool enable)
{
  m_dlg->cbRunInTerminal->setChecked(enable);
  m_autoCheckedRunInTerm = false;

  if (enable)
  {
    m_prevIconName = m_iconName;
    m_iconName = QLatin1String( "konsole" );
    setIcon();
  }
  else if (!m_prevIconName.isEmpty())
  {
      m_iconName = m_prevIconName;
      setIcon();
  }
}

void Minicli::slotChangeUid(bool enable)
{
  m_dlg->leUsername->setEnabled(enable);
  m_dlg->lbUsername->setEnabled(enable);

  if(enable)
  {
    m_dlg->leUsername->selectAll();
    m_dlg->leUsername->setFocus();
  }

  updateAuthLabel();
}

void Minicli::slotChangeScheduler(bool enable)
{
  m_dlg->slPriority->setEnabled(enable);
  m_dlg->lbLowPriority->setEnabled(enable);
  m_dlg->lbHighPriority->setEnabled(enable);

  updateAuthLabel();
}

bool Minicli::needsKDEsu()
{
  return ((m_dlg->cbPriority->isChecked() && ((m_iPriority > 50) ||
          (m_iScheduler != StubProcess::SchedNormal))) ||
          (m_dlg->cbRunAsOther->isChecked() && !m_dlg->leUsername->text().isEmpty()));
}

void Minicli::slotRealtime(bool enabled)
{
  m_iScheduler = enabled ? StubProcess::SchedRealtime : StubProcess::SchedNormal;

  if (enabled)
  {
    if (KMessageBox::warningContinueCancel(this,
                i18n("Running a realtime application can be very dangerous. "
                    "If the application misbehaves, the system might hang "
                    "unrecoverably.\nAre you sure you want to continue?"),
                i18n("Warning - Run Command"), KGuiItem(i18n("&Run Realtime")),QString::null,KMessageBox::Notify|KMessageBox::PlainCaption)
        != KMessageBox::Continue )
    {
      m_iScheduler = StubProcess::SchedNormal;
      m_dlg->cbRealtime->setChecked(false);
    }
  }

  updateAuthLabel();
}

void Minicli::slotPriority(int priority)
{
  // Provide a way to easily return to the default priority
  if ((priority > 40) && (priority < 60))
  {
    priority = 50;
    m_dlg->slPriority->setValue(50);
  }

  m_iPriority = priority;

  updateAuthLabel();
}

QString Minicli::calculate(const QString &exp)
{
   QString result;
   QString cmd = QString("echo $((%1))").arg(exp);
   FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
   if (fs)
   {
      QTextStream ts(fs, QIODevice::ReadOnly);
      result = ts.read().trimmed();
      pclose(fs);
   }
   return result;
}

void Minicli::fontChange( const QFont & )
{
   setMaxCommandBoxWidth();
   adjustSize();
   setFixedSize(size());
}

// vim: set et ts=2 sts=2 sw=2:

