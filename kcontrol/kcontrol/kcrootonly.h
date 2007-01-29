
#ifndef _kcontrol_kcrootonly_
#define _kcontrol_kcrootonly_

#include <kcmodule.h>
class KComponentData;

class KCRootOnly: public KCModule {
public:
	KCRootOnly(const KComponentData &inst, QWidget *parent);
};

#endif
