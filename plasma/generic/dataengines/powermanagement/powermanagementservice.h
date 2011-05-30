#ifndef POWERMANAGEMENT_SERVICE_H
#define POWERMANAGEMENT_SERVICE_H

#include <Plasma/Service>

class PowermanagementEngine;

class PowermanagementService : public Plasma::Service
{
    Q_OBJECT

public:
    PowermanagementService(PowermanagementEngine* parent, const QString& source);

protected:
    Plasma::ServiceJob* createJob(const QString& operation,
                  QMap<QString, QVariant>& parameters);

private:
    PowermanagementEngine* m_engine;
    QString m_dest;
};

#endif // POWERMANAGEMENT_SERVICE_H

