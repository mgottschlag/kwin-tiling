//-----------------------------------------------------------------------------
//
// KDE Display module base
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <qwidget.h>

#include <kcmodule.h>

class KDisplayModule : public KCModule
{
	Q_OBJECT
public:
	KDisplayModule( QWidget *parent, int mode, int desktop = 0 );

	virtual void readSettings( int deskNum = 0 ) = 0;
	virtual void apply( bool ) = 0;

	int runMode()
		{ return _runMode; }

public:
	enum { Init, Setup };

private:
	int _runMode;
};

#endif

