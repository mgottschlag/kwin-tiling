//-----------------------------------------------------------------------------
//
// KDE display energy saving setup module
//
// Written by:	Tom Vijlbrief 1999
//

#ifndef __ENERGY_H__
#define __ENERGY_H__

#include <qwidget.h>
#include <qlined.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qchkbox.h>
#include <kslider.h>
#include <kcontrol.h>

#include "display.h"

class QPushButton;
class QLCDNumber;

class KEnergy : public KDisplayModule
{
	Q_OBJECT
	
public:
	KEnergy(QWidget *parent, Mode m);
	~KEnergy();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	Display *kde_display;

protected slots:
	void slotChangeEnable();
	void slotChangeStandby();
	void slotChangeSuspend();
	void slotChangeOff();

protected:
	void writeSettings();
	void setDefaults();
	
protected:
	QCheckBox *cbEnable;
	QSlider *standbySlider;
	QSlider *suspendSlider;
	QSlider *offSlider;
	class QLCDNumber *standbyLCD;
	class QLCDNumber *suspendLCD;
	class QLCDNumber *offLCD;
	bool changed;
	int standby;
	int suspend;
	int off;
	bool enabled;
	bool hasDPMS;
};

#endif

