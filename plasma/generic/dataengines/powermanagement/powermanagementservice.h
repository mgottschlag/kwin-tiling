#ifndef POWERMANAGEMENT_SERVICE_H
#define POWERMANAGEMENT_SERVICE_H

#include "powermanagementengine.h"

#include <Plasma/Service>
#include <Plasma/ServiceJob>

using namespace Plasma;

class PowermanagementEngine;

class PowermanagementService : public Plasma::Service
{
	Q_OBJECT

public:
	PowermanagementService(PowermanagementEngine* parent, QStringList sources, const QString& source);

protected:
	ServiceJob* createJob(const QString& operation,
			      QMap<QString, QVariant> &parameters);

private:
	PowermanagementEngine* m_powermanagementEngine;

};

#endif // POWERMANAGEMENT_SERVICE_H

