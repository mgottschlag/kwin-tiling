#ifndef classFileSelector_included
#define classFileSelector_included

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
// QDir has errors in it???
// #include <qfiledialog.h>

class classFileSelector : public QWidget
{
    Q_OBJECT

public:

    classFileSelector( QWidget* parent = NULL, const char* name = NULL );
    ~classFileSelector();

	QLineEdit	*pLineEdit;
	QPushButton	*pButton;

protected slots:
	void pButton_Clicked();

protected:

};
#endif
