
#include "display.h"

#include "display.moc"

KDisplayModule::KDisplayModule(QWidget *parent, Mode mode)
	: KConfigWidget( parent )
{
    mMode = mode;
}

