
#include "display.h"

#include "display.moc"

KDisplayModule::KDisplayModule( QWidget *parent, int mode, int )
	: KCModule( parent , "displaymodule" )
{
	_runMode = mode;
}

