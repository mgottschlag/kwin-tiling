#include <sys/types.h>
#include <dcopobject.h>

#ifndef __AppStarting_h__
#define __AppStarting_h__

class QString;

class AppStarting : virtual public DCOPObject
{
    K_DCOP
k_dcop:
    virtual void clientStarted(QString name, QString icon, pid_t pid, QString bin, bool compliant) = 0;
    virtual void clientDied(pid_t pid) = 0;
};

#endif
