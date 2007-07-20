/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "sensorshellagent.h"

#include <kdebug.h>
#include <k3process.h>

//#include "sensorclient.h"
#include "sensormanager.h"

using namespace KSGRD;

SensorShellAgent::SensorShellAgent( SensorManager *sm )
  : SensorAgent( sm ), m_daemon( 0 )
{
}

SensorShellAgent::~SensorShellAgent()
{
  if ( m_daemon ) {
    m_daemon->writeStdin( "quit\n", strlen( "quit\n" ) );
    delete m_daemon;
    m_daemon = 0;
  }
}
	
bool SensorShellAgent::start( const QString &host, const QString &shell,
                              const QString &command, int )
{
  m_daemon = new K3Process;
  m_daemon->setUseShell(true);
  m_retryCount=3;
  setHostName( host );
  m_shell = shell;
  m_command = command;

  connect( m_daemon, SIGNAL( processExited( K3Process* ) ),
           SLOT( daemonExited( K3Process* ) ) );
  connect( m_daemon, SIGNAL( receivedStdout( K3Process*, char*, int ) ),
           SLOT( msgRcvd( K3Process*, char*, int ) ) );
  connect( m_daemon, SIGNAL( receivedStderr( K3Process*, char*, int ) ),
           SLOT( errMsgRcvd( K3Process*, char*, int ) ) );
  connect( m_daemon, SIGNAL( wroteStdin( K3Process* ) ),
           SLOT( msgSent( K3Process* ) ) );

  QString cmd;
  if ( !command.isEmpty() )
    cmd =  command;
  else
    cmd = m_shell + ' ' + hostName() + " ksysguardd";
  *m_daemon << cmd;

  if ( !m_daemon->start( K3Process::NotifyOnExit, K3Process::All ) ) {
    sensorManager()->hostLost( this );
    kDebug (1215) << "Command '" << cmd << "' failed"  << endl;
    return false;
  }

  return true;
}

void SensorShellAgent::hostInfo( QString &shell, QString &command,
                                 int &port) const
{
  shell = m_shell;
  command = m_command;
  port = -1;
}

void SensorShellAgent::msgSent( K3Process* )
{
  setTransmitting( false );

	// Try to send next request if available.
  executeCommand();
}

void SensorShellAgent::msgRcvd( K3Process*, char *buffer, int buflen )
{
  if ( !buffer || buflen == 0 )
    return;
  m_retryCount = 3; //we received an answer, so reset our retry count back to 3
  processAnswer( buffer, buflen );
}

void SensorShellAgent::errMsgRcvd( K3Process*, char *buffer, int buflen )
{
  if ( !buffer || buflen == 0 )
    return;

  QString buf = QString::fromUtf8( buffer, buflen );

  kDebug(1215) << "SensorShellAgent: Warning, received text over stderr!"
                << endl << buf << endl;
}

void SensorShellAgent::daemonExited( K3Process * )
{
  if ( m_retryCount-- <= 0 || !m_daemon->start( K3Process::NotifyOnExit, K3Process::All ) ) {
    setDaemonOnLine( false );
    sensorManager()->hostLost( this );
    sensorManager()->requestDisengage( this );
  }
}

bool SensorShellAgent::writeMsg( const char *msg, int len )
{
  return m_daemon->writeStdin( msg, len );
}

bool SensorShellAgent::txReady()
{
  return !transmitting();
}

#include "sensorshellagent.moc"
