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

#include "classProperties.h"

class classDrivers : public QWidget
{
    Q_OBJECT

public:

    classDrivers( QWidget* parent = NULL, const char* name = NULL );
    ~classDrivers();

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
