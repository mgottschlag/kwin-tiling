/*

  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            



#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h> 


#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>


#include <kprocess.h>
#include <kapp.h>
#include <dcopclient.h>


#include "config.h"		/* needed by HP-UX */
#include "proxymodule.h"
#include "proxymodule.moc"
#include "utils.h"
#include "config.h"


#include <X11/Xlib.h>


ProcessProxy::ProcessProxy(QString exec, bool onlyRoot)
  : _process(0), _running(false)
{
  QStringList args;
  splitString(exec, ' ', args);

  if (args.count() < 1)
    return;

  // create the process
  _process = new KProcess;
  connect(_process, SIGNAL(processExited(KProcess*)), 
	  this, SLOT(processTerminated(KProcess*)));

  // find out if we have to call kdesu
  if (onlyRoot && (getuid() != 0))
    *_process << "kdesu" << "root" << "-c";

  // feed the arguments to the process.
  // TODO: This ignores escaping!!!
  QStringList::Iterator it;
  for (it = args.begin(); it != args.end(); ++it)
    *_process << *it;

  // now start the process
  if (!_process->start())
    return;

  _running = true;
}


ProcessProxy::~ProcessProxy()
{
   delete _process;
}


void ProcessProxy::processTerminated(KProcess *proc)
{
  if (proc == _process)
    {
      emit terminated();
      _process = 0;
    }
}


bool ChangeNotifier::process(const QCString &fun, const QByteArray &data,
			     QCString& replyType, QByteArray &replyData)
{
  if (fun == "changed(int)")
    {
      int state;

      replyType = "void";
      QDataStream in(data, IO_ReadOnly);
      in >> state;

      emit changed(state != 0);

      return true;
    }

  return false;
}


DCOPProxy::DCOPProxy(QWidget *parent, const ModuleInfo &mod)
  : KCModule(parent), _proxy(0), _process(0), _embed(0), _running(false),
    _notify(0)
{
  // find out the DCOP server to use
  QString server;
  QString fName = ::getenv("HOME");
  fName += "/.DCOPserver";
  QFile f(fName);
  if (f.open(IO_ReadOnly)) 
    {
      QTextStream t(&f);
      server = t.readLine().latin1();
      f.close();                                                                
    }

  // set the name of the ICE authority file
  QString auth = ::getenv("ICEAUTHORITY");
  if (auth.isEmpty())
    {
      auth = QString(::getenv("HOME"))+"/.ICEauthority";
      ::setenv("ICEAUTHORITY", auth.ascii(), 1);
    }

  // run the remote process
  _process = new KProcess;
  connect(_process, SIGNAL(processExited(KProcess*)), 
	  this, SLOT(processTerminated(KProcess*)));
  *_process << "kdesu" << "root" << "-c" << "kcmroot" << mod.fileName();
  if (!server.isEmpty())
    *_process << "-dcopserver" << server;
  if (!_process->start())
    {
      _process = 0;
      return;
    }
   
  // wait for the remote process to register
  // after 30 seconds, assume it failed
  int cnt=0;
  while (!kapp->dcopClient()->isApplicationRegistered(mod.moduleId()))
    {
      sleep(1);
      cnt++;
      if (cnt >= 30)
	{
	  _process->kill();
	  delete _process;
	  return;
	}
    }

  // create the callback object
  _notify = new ChangeNotifier(mod.moduleId()+"-server");
  connect(_notify, SIGNAL(changed(bool)), this, SLOT(remoteChanged(bool)));

  // setup the proxy object
  _proxy = new KCModuleIface_stub(mod.moduleId(), "KCModuleIface");
  int winId = _proxy->winId();

  // swallow the window
  _embed = new QXEmbed(this);
  resizeEvent(0);
  _embed->embed(winId);

  _running = true;
}


DCOPProxy::~DCOPProxy()
{
  delete _proxy;
  delete _process;
  delete _notify;
}


void DCOPProxy::resizeEvent(QResizeEvent *)
{
  if (_embed)
    _embed->resize(width(), height());
}


void DCOPProxy::processTerminated(KProcess *proc)
{
  if (proc == _process)
    {
      _process = 0;
    }
}


void DCOPProxy::remoteChanged(bool state)
{
  emit changed(state);
}
