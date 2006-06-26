#ifndef KSMSERVER_INTERFACE_H
#define KSMSERVER_INTERFACE_H

#include <dcopobject.h>
#include <QStringList>

class KSMServerInterface : public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KSMServerInterface")

public Q_SLOTS:
  Q_SCRIPTABLE void logout(int, int, int );
  Q_SCRIPTABLE QStringList sessionList();

  Q_SCRIPTABLE QString currentSession();
  Q_SCRIPTABLE void saveCurrentSession();
  Q_SCRIPTABLE void saveCurrentSessionAs( QString );

  Q_SCRIPTABLE void suspendStartup( QString );
  Q_SCRIPTABLE void resumeStartup( QString );
};

#endif
