//-----------------------------------------------------------------------------
//
// KDE display energy saving setup module
//
// Written by:	Tom Vijlbrief 1999
//

#ifndef __ENERGY_H__
#define __ENERGY_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qchkbox.h>
#include <kspinbox.h>
#include <kslider.h>
#include <kcontrol.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "display.h"


class KEnergy : public KDisplayModule
{
	Q_OBJECT
	
public:
	KEnergy( QWidget *parent, int mode, int desktop = 0 );
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
	void slotApply();
	void slotHelp();

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
	Bool changed;
	int standby;
	int suspend;
	int off;
	Bool enabled;
	Bool hasDPMS;
};

#endif

