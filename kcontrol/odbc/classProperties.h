#ifndef classProperties_included
#define classProperties_included

#include <qdialog.h>

class QComboBox;

#include <odbcinstext.h>

#include "classFileSelector.h"

class classProperties : public QDialog
{
    Q_OBJECT

public:
    classProperties( QWidget* parent = NULL, const char* name = NULL, HODBCINSTPROPERTY hTheFirstProperty = NULL );
    ~classProperties();

	/* void resizeEvent( QResizeEvent *p ); */

private:
	int					nProperties;
	HODBCINSTPROPERTY	hFirstProperty;

	void setCurrentItem( QComboBox *pComboBox, char *pszItem );

protected slots:

	void pbOk_Clicked();
	void pbCancel_Clicked();

};

#endif
