#ifndef classSystemDSN_included
#define classSystemDSN_included

#include <qwidget.h>
#include <qpushbt.h>

#include <kconfig.h>
#include <kcontrol.h>

#include "classDSNList.h"

class classSystemDSN : public KConfigWidget
{
    Q_OBJECT

public:

    classSystemDSN( QWidget* parent = NULL, const char* name = NULL );
    ~classSystemDSN();

	virtual void applySettings();
	virtual void loadSettings();

protected:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
    QPushButton* pbConfigure;
    classDSNList* pDSNList;

};
#endif
