

#ifndef SYSTEMTRAYDAEMON
#define SYSTEMTRAYDAEMON

#include <kdedmodule.h>

#include <qobject.h>

class SystemTrayDaemon : public KDEDModule
{
Q_OBJECT
public:
    SystemTrayDaemon(QObject *parent, const QList<QVariant>&);
    ~SystemTrayDaemon();
};
#endif
