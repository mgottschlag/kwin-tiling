//-----------------------------------------------------------------------------
//
// KDE Display module base
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <qwidget.h>

#include <kcontrol.h>

class KDisplayModule : public KConfigWidget
{
	Q_OBJECT
public:
	enum Mode { Init, Setup };
	KDisplayModule(QWidget *parent, Mode mode);

    Mode mode() const { return mMode; }

private:
    Mode mMode;
};

#endif

