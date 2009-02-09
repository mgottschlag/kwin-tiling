

#ifndef SYSTEMTRAYDAEMON
#define SYSTEMTRAYDAEMON

#include <kdedmodule.h>

#include <QObject>
#include <QStringList>

class SystemTrayDaemon : public KDEDModule
{
Q_OBJECT
public:
    SystemTrayDaemon(QObject *parent, const QList<QVariant>&);
    ~SystemTrayDaemon();

public Q_SLOTS:
    void registerService(const QString &service);

    QStringList registeredServices() const;

Q_SIGNALS:
    void serviceRegistered(const QString &service);

private:
    QStringList m_registeredServices;
};
#endif
