#include "powermanagementservice.h"
#include "powermanagementjob.h"
#include "powermanagementengine.h"

PowermanagementService::PowermanagementService (PowermanagementEngine* parent, const QString& source)
    : Plasma::Service(parent),
      m_engine (parent),
      m_dest (source)
{
    setName ("powermanagement");
    setDestination (source);
}

Plasma::ServiceJob* PowermanagementService::createJob (const QString& operation,
                                                       QMap<QString, QVariant>& parameters)
{
    return new PowermanagementJob (m_engine, m_dest, operation, parameters, this);
}

#include "powermanagementservice.moc"
