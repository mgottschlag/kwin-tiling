#ifndef classUserDSN_included
#define classUserDSN_included

#include <qwidget.h>
#include <qpushbt.h>

#include <kconfig.h>
#include <kcontrol.h>

#include "classDSNList.h"

class classUserDSN : public KConfigWidget
{
    Q_OBJECT

public:

    classUserDSN( QWidget* parent = NULL, const char* name = NULL );
    ~classUserDSN();

	virtual void applySettings();
	virtual void loadSettings();

protected:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
    QPushButton* pbConfigure;
    classDSNList* pDSNList;

};
#endif
