#ifndef KSMSERVER_INTERFACE_H
#define KSMSERVER_INTERFACE_H

#include <dcopobject.h>
#include <qstringlist.h>

class KSMServerInterface : virtual public DCOPObject
{
  K_DCOP

k_dcop:
  virtual void logout(int, int, int ) = 0;
  virtual void restoreSessionInternal() = 0;
  virtual void restoreSessionDoneInternal() = 0;
  virtual QStringList sessionList() = 0;

  virtual QString currentSession() = 0;
  virtual void saveCurrentSession() = 0;
  virtual void saveCurrentSessionAs( QString ) = 0;

  virtual void autoStart2() = 0;
  
  virtual void suspendStartup() = 0;
  virtual void resumeStartup() = 0;
};

#endif
