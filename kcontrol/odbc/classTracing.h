#ifndef CLASSTRACING_H
#define CLASSTRACING_H

#include <odbcinstext.h>

#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpixmap.h>

#include "classFileSelector.h"

class classTracing : public QWidget
{
	Q_OBJECT
public:
	classTracing( QWidget* parent = NULL, const char* name = NULL );
	virtual ~classTracing();

	QLabel				*plabelTracing;
	QCheckBox			*pTracing;
	classFileSelector		*pTraceFile;
	QPushButton			*pApply;

public slots:
	void apply();

protected slots:

protected:

};

#endif


