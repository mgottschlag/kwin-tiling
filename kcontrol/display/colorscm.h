//-----------------------------------------------------------------------------
//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include <qlistbox.h>
#include <qslider.h>

#include "kcolorbtn.h"
#include "savescm.h"
#include "widgetcanvas.h"

#include "display.h"
#include <X11/X.h>
#include <kcontrol.h>
#include <kconfig.h>

extern bool runResourceManager;

class KColorScheme : public KDisplayModule
{
	Q_OBJECT
public:
	KColorScheme( QWidget *parent, Mode mode);
	~KColorScheme();
	
	virtual void readSettings( int ) {};
	virtual void apply();
	virtual void applySettings();
	virtual void loadSettings();
	virtual void defaultSettings();
	
	QColor colorPushColor;
	
    QPalette createPalette();
	
protected slots:
	void slotPreviewScheme( int );
	void slotWidgetColor( int );
	void slotSelectColor( const QColor &col );
	void slotColorForWidget( int, const QColor &);
	void slotSave();
	void slotAdd();
	void slotRemove();
	void sliderValueChanged( int val );
	void resizeEvent( QResizeEvent * );

protected:
	void writeSettings();
	void writeNamedColor( KConfigBase *config,
				const char *key, const char *name );
	void readSchemeNames();
	void readScheme( int index = 0 );
	void writeScheme();
	void setDefaults();

	
protected:
	QSlider *sb;
	QComboBox *wcCombo;
	KColorButton *colorButton;
	WidgetCanvas *cs;
	QPushButton *saveBt;
	QPushButton *addBt;
	QPushButton *removeBt;
	QListBox *sList;
	QStrList *schemeList;
	QStrList *sFileList;
	QString schemeFile;
	int nSysSchemes;
	
	bool changed;
	bool useRM;
};

#endif

