#ifndef classDrivers_included
#define classDrivers_included

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <ini.h>
#include <odbcinstext.h>

#include <qwidget.h>
#include <qlistview.h>
#include <qpushbt.h>

#include <kconfig.h>
#include <kcontrol.h>

#include "classProperties.h"

class classDrivers : public KConfigWidget
{
    Q_OBJECT

public:

    classDrivers( QWidget* parent = NULL, const char* name = NULL );
    ~classDrivers();

	virtual void applySettings();
	virtual void loadSettings();

	QListView *getListView() { return lvwDrivers; };

public slots:
	void Add();
	void Edit();
	void Delete();

protected:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
    QPushButton* pbConfigure;
    QListView* lvwDrivers;

private:
	HINI	hIni;
	char 	szINI[FILENAME_MAX+1];

	void	Load();
	void	FreeProperties( HODBCINSTPROPERTY *hFirstProperty );

};
#endif
