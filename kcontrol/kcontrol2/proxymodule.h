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



#ifndef __PROXYMODULE_H__
#define __PROXYMODULE_H__


#include <qstring.h>
#include <qxembed.h>


#include <dcopobject.h>
#include <kcmodule.h>
class KProcess;


#include "moduleinfo.h"
#include "KCModuleIface_stub.h"


class ProcessProxy : public QObject
{
  Q_OBJECT

public:

  ProcessProxy(QString exec, bool onlyRoot=false);
  ~ProcessProxy();

  bool running() { return _running; };


signals:

  void terminated();


private slots:

  void processTerminated(KProcess *proc);
    

private:

  KProcess *_process;
  bool     _running;

};


class ChangeNotifier : public QObject, virtual public DCOPObject
{
  Q_OBJECT

public:

  ChangeNotifier(QCString objid) : QObject(), DCOPObject(objid) {};

  bool process(const QCString &fun, const QByteArray &data,
	       QCString& replyType, QByteArray &replyData);


signals:

  void changed(bool state);

};


class DCOPProxy : public KCModule
{
  Q_OBJECT

public: 

  DCOPProxy(QWidget *parent, const ModuleInfo &mod);
  ~DCOPProxy();

  void load() { _proxy->load(); };
  void save() { _proxy->save(); };
  void defaults() { _proxy->defaults(); };
 
  int buttons() { return _proxy->buttons(); };

  bool running() { return _running; };


protected:  

  void resizeEvent(QResizeEvent *event);


private slots:

  void processTerminated(KProcess *proc);
  void remoteChanged(bool state);
 

private:

  KCModuleIface_stub *_proxy;
  KProcess           *_process;
  QXEmbed            *_embed;
  bool               _running;

  ChangeNotifier     *_notify;

};


#endif
