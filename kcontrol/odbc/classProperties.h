#ifndef classProperties_included
#define classProperties_included

#include <qdialog.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qtooltip.h>

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
	QBoxLayout 			*pTopLayout;
	QGridLayout			*pGridLayout;
	HODBCINSTPROPERTY	hFirstProperty;

	void setCurrentItem( QComboBox *pComboBox, char *pszItem );

protected slots:

	void pbOk_Clicked();
	void pbCancel_Clicked();

};

#endif
