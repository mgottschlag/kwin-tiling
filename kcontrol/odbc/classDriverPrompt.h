#ifndef classDriverPrompt_included
#define classDriverPrompt_included

#include <qdialog.h>
#include <qframe.h>

#include "classDrivers.h"

class classDriverPrompt : public QDialog
{
    Q_OBJECT

public:
    classDriverPrompt( QWidget* parent = NULL, const char* name = NULL, bool modal=FALSE );
    ~classDriverPrompt();

	QString	qsDriverName;
	QString	qsDescription;
	QString	qsDriver;
	QString	qsSetup;

protected slots:

    void pbCancel_Clicked();
    void pbOk_Clicked();

protected:
    classDrivers* pDrivers;

};
#endif
