//-----------------------------------------------------------------------------
//
// KDE display energy saving setup module
//
// Written by:	Tom Vijlbrief 1999
//

#ifndef __ENERGY_H__
#define __ENERGY_H__

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <kcontrol.h>

#include "display.h"

class QPushButton;
class QCheckBox;

class KIntNumInput;

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
	KIntNumInput *standbySlider;
	KIntNumInput *suspendSlider;
	KIntNumInput *offSlider;
	bool changed;
	int standby;
	int suspend;
	int off;
	bool enabled;
	bool hasDPMS;
};

#endif

