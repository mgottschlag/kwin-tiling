#ifndef SAVESCM_H
#define SAVESCM_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class SaveScm : public QDialog {
	Q_OBJECT
public:

	SaveScm( QWidget *parent, const char *name );
	
	QLineEdit* nameLine;
	QPushButton* ok;
	QPushButton* cancel;
};

#endif
