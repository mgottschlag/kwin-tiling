#ifndef classDSNList_included
#define classDSNList_included

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <ini.h>
#include <odbcinstext.h>

#include <qmessagebox.h>
#include <qwidget.h>
#include <qlistview.h>
#include <qstring.h>

#include "classDriverPrompt.h"
#include "classProperties.h"
#include "classDLL.h"

class classDSNList : public QListView
{
    Q_OBJECT

public:
    classDSNList( QWidget* parent = NULL, const char* name = NULL );
    ~classDSNList();
	
	void Load( int nSource );

public slots:
	void Add();
	void Edit();
	void Delete();

private:
	int nSource;
};
#endif

