
#ifndef _kcontrol_kcrootonly_
#define _kcontrol_kcrootonly_

#include <kcmodule.h>
class KInstance;

class KCRootOnly: public KCModule {
public:
	KCRootOnly(KInstance *inst, QWidget *parent);
};

#endif
