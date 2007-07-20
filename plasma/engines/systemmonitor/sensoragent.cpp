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

//#include <stdlib.h>

#include "sensoragent.h"

#include <kdebug.h>
#include <klocale.h>

#include "sensorclient.h"
#include "sensormanager.h"

/**
  This can be used to debug communication problems with the daemon.
  Should be set to 0 in any production version.
*/
#define SA_TRACE 0

using namespace KSGRD;

SensorAgent::SensorAgent( SensorManager *sm )
  : m_sensorManager( sm )
{
  m_daemonOnLine = false;
  m_transmitting = false;
  m_foundError = false;
}

SensorAgent::~SensorAgent()
{
}

void SensorAgent::sendRequest( const QString &req, SensorClient *client)
{
  SensorRequest *sensorreq = 0;
  for(int i =0; i < m_inputFIFO.size(); ++i) {
    sensorreq = m_inputFIFO.at(i);
    if(client == sensorreq->client() && req == sensorreq->request()) {
      executeCommand();
      return; //don't bother to resend the same request if we already have it in our queue to send
    }
  }
  for(int i =0; i < m_processingFIFO.size(); ++i) {
    sensorreq = m_processingFIFO.at(i);
    if(client == sensorreq->client() && req == sensorreq->request())
      return; //don't bother to resend the same request if we have already sent the request to client and just waiting for an answer
  }

  /* The request is registered with the FIFO so that the answer can be
   * routed back to the requesting client. */
  m_inputFIFO.enqueue( new SensorRequest( req, client ) );

#if SA_TRACE
  kDebug(1215) << "-> " << req << "(" << m_inputFIFO.count() << "/"
                << m_processingFIFO.count() << ")" << endl;
#endif
  executeCommand();
}

void SensorAgent::processAnswer( const char *buf, int buflen )
{
  //It is possible for an answer/error message  to be split across multiple processAnswer calls.  This makes our life more difficult
  //We have to keep track of the state we are in.  Any characters that we have not parsed yet we put in
  //m_leftOverBuffer
  QByteArray buffer = QByteArray::fromRawData(buf, buflen);
  
  if(!m_leftOverBuffer.isEmpty()) {
	buffer = m_leftOverBuffer + buffer; //If we have data left over from a previous processAnswer, then we have to prepend this on
	m_leftOverBuffer.clear();
  }
  
#if SA_TRACE
  kDebug(1215) << "<- " << QString::fromUtf8(buffer, buffer.size()) << endl;
#endif
  int startOfAnswer = 0;  //This can become >= buffer.size(), so check before using!
  for ( int i = 0; i < buffer.size(); i++ ) {
    if ( buffer.at(i) == '\033' ) {  // 033 in octal is the escape character.  The signifies the start of an error
      //The first time we see 033 we simply set m_foundError to true
      //Then the error message will come, and then we will receive another escape character.
      
      m_foundError = !m_foundError;
      if ( !m_foundError ) {  //We found the end of the error
	//Piece together the error from what we read now, and what we read last time processAnswer was called
	QString error = QString::fromUtf8(buffer.constData() + startOfAnswer, i-startOfAnswer);
        if ( error == "RECONFIGURE" )
          emit reconfigure( this );
        else {
          /* We just received the end of an error message, so we
           * can display it. */
          SensorMgr->notify( i18n( "Message from %1:\n%2" ,
                             m_hostName ,
                             error ) );
        }
      }
      m_answerBuffer.clear();
      startOfAnswer = i+1;
      continue;
    }

    //The spec was supposed to be that it returned "\nksysguardd> " but some seem to forget the space, so we have to compensate.  Sigh 
    if( (i==0 && buffer.size() >= (signed)(sizeof("ksysguardd>"))-1 && qstrncmp(buffer.constData(), "ksysguardd>", sizeof("ksysguardd>")-1) == 0) ||
	(buffer.size() -i >= (signed)(sizeof("\nksysguardd>")) -1 && qstrncmp(buffer.constData()+i, "\nksysguardd>", sizeof("\nksysguardd>")-1) == 0)) {

	QByteArray answer(buffer.constData()+startOfAnswer, i-startOfAnswer);
	if(!answer.isEmpty())
		m_answerBuffer << answer;
#if SA_TRACE
	kDebug(1215) << "<= " << m_answerBuffer
		<< "(" << m_inputFIFO.count() << "/"
		<< m_processingFIFO.count() << ")" << endl;
#endif
	if(buffer.at(i) == '\n')
		i++;
	i += sizeof("ksysguardd>") -2;  //Move i on to the next answer (if any). -2 because sizeof adds one for \0  and the for loop will increment by 1 also
	if(i+1 < buffer.size() && buffer.at(i+1) == ' ') i++;
	startOfAnswer = i+1;

	//We have found the end of one reply
	if ( !m_daemonOnLine ) {
		/* First '\nksysguardd> ' signals that the daemon is
	  	 * ready to serve requests now. */
		m_daemonOnLine = true;
#if SA_TRACE
		kDebug(1215) << "Daemon now online!" << endl;
#endif
		m_answerBuffer.clear();
		continue;
	}

	//Deal with the answer we have now read in

	// remove pending request from FIFO
	if ( m_processingFIFO.isEmpty() ) {
		kDebug(1215)	<< "ERROR: Received answer but have no pending "
				<< "request!" << endl;
		m_answerBuffer.clear();
		continue;
	}
		
	SensorRequest *req = m_processingFIFO.dequeue();
	// we are now responsible for the memory of req - we must delete it!
	if ( !req->client() ) {
		/* The client has disappeared before receiving the answer
		 * to his request. */
		delete req;
		m_answerBuffer.clear();
		continue;
	}
		
	if(!m_answerBuffer.isEmpty() && m_answerBuffer[0] == "UNKNOWN COMMAND") {
		/* Notify client that the sensor seems to be no longer available. */
        kDebug() << "Received UNKNOWN COMMAND for: " << req->request() << endl; 
		req->client()->sensorLost( req->request() );
	} else {
		// Notify client of newly arrived answer.
		req->client()->answerReceived( req->request(), m_answerBuffer );
	}
	delete req;
	m_answerBuffer.clear();
    } else if(buffer.at(i) == '\n'){
	m_answerBuffer << QByteArray(buffer.constData()+startOfAnswer, i-startOfAnswer);
	startOfAnswer = i+1;
    }
  }

  m_leftOverBuffer += QByteArray(buffer.constData()+startOfAnswer, buffer.size()-startOfAnswer);
  executeCommand();
}

void SensorAgent::executeCommand()
{
  /* This function is called whenever there is a chance that we have a
   * command to pass to the daemon. But the command may only be sent
   * if the daemon is online and there is no other command currently
   * being sent. */
  if ( m_daemonOnLine && txReady() && !m_inputFIFO.isEmpty() ) {
    SensorRequest *req = m_inputFIFO.dequeue();

#if SA_TRACE
    kDebug(1215) << ">> " << req->request().toAscii() << "(" << m_inputFIFO.count()
                  << "/" << m_processingFIFO.count() << ")" << endl;
#endif
    // send request to daemon
    QString cmdWithNL = req->request() + '\n';
    if ( writeMsg( cmdWithNL.toLatin1(), cmdWithNL.length() ) )
      m_transmitting = true;
    else
      kDebug(1215) << "SensorAgent::writeMsg() failed" << endl;

    // add request to processing FIFO.
    // Note that this means that m_processingFIFO is now responsible for managing the memory for it.
    m_processingFIFO.enqueue( req );
  }
}

void SensorAgent::disconnectClient( SensorClient *client )
{
  for (int i = 0; i < m_inputFIFO.size(); ++i)
    if ( m_inputFIFO[i]->client() == client )
      m_inputFIFO[i]->setClient(0);
  for (int i = 0; i < m_processingFIFO.size(); ++i)
    if ( m_processingFIFO[i]->client() == client )
      m_processingFIFO[i]->setClient( 0 );
  
}

SensorManager *SensorAgent::sensorManager()
{
  return m_sensorManager;
}

void SensorAgent::setDaemonOnLine( bool value )
{
  m_daemonOnLine = value;
}

bool SensorAgent::daemonOnLine() const
{
  return m_daemonOnLine;
}

void SensorAgent::setTransmitting( bool value )
{
  m_transmitting = value;
}

bool SensorAgent::transmitting() const
{
  return m_transmitting;
}

void SensorAgent::setHostName( const QString &hostName )
{
  m_hostName = hostName;
}

const QString &SensorAgent::hostName() const
{
  return m_hostName;
}


SensorRequest::SensorRequest( const QString &request, SensorClient *client )
  : m_request( request ), m_client( client )
{
}

SensorRequest::~SensorRequest()
{
}

void SensorRequest::setRequest( const QString &request )
{
  m_request = request;
}

QString SensorRequest::request() const
{
  return m_request;
}

void SensorRequest::setClient( SensorClient *client )
{
  m_client = client;
}

SensorClient *SensorRequest::client()
{
  return m_client;
}

#include "sensoragent.moc"
