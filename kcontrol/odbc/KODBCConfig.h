#include <stdlib.h>

#include <qobject.h>
#include <kcmodule.h>
#include <qtabwidget.h>

#include "classUserDSN.h"
#include "classSystemDSN.h"
#include "classDrivers.h"
#include "classTracing.h"
#include "classAbout.h"

class KODBCConfig : public KCModule
{
	Q_OBJECT
public:

	KODBCConfig( QWidget *pParent, const char *pszName);

	virtual void load();
	virtual void save();
	virtual void defaults();

signals:
	void changed( bool );


private:
	QTabWidget	*pTabs;
	classUserDSN    *pUserDSN;
	classSystemDSN  *pSystemDSN;
	classDrivers    *pDrivers;
	classTracing	*pTracing;
	classAbout      *pAbout;

};


