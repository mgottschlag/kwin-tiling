//
// KDE Energy saver setup module
//
// Written by: Tom Vijlbrief 1999
//

#include <qcheckbox.h>
#include <qlayout.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <klocale.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "energy.h"
#include "energy.moc"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_DPMS
#include <X11/extensions/dpms.h>

extern "C" int DPMSQueryExtension(Display *, int *, int *);
extern "C" void DPMSEnable(Display *);
extern "C" void DPMSDisable(Display *);
extern "C" void DPMSSetTimeouts(Display *, int, int, int);
#endif

extern int dropError(Display *, XErrorEvent *);

static const int DFLT_STANDBY	= 0;
static const int DFLT_SUSPEND	= 30;
static const int DFLT_OFF	= 60;

//------------------------------------------------------------------

KEnergy::KEnergy(QWidget *parent, Mode m)
	: KDisplayModule(parent, m)
{
	changed= false;
	enabled= false;

	standby= DFLT_STANDBY;
	suspend= DFLT_SUSPEND;
	off= DFLT_OFF;

	readSettings();

	kde_display = x11Display();
    hasDPMS = false;

#if HAVE_DPMS
    int dummy;
    hasDPMS= DPMSQueryExtension(kde_display, &dummy, &dummy);
#endif

	apply(true);

	// if we are just initialising we don't need to create setup widget
	if (mode() == Init)
		return;

	setName( i18n("Energy").ascii() );
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	QBoxLayout *h1Layout= new QHBoxLayout();

	cbEnable= new QCheckBox( i18n( "&Enable Display Energy Saving" ),
				 this );
	cbEnable->setChecked(enabled);
	cbEnable->setEnabled(hasDPMS);
	connect(cbEnable, SIGNAL(clicked()), SLOT(slotChangeEnable()));

	QLabel *pixlabel= new QLabel(this);
	pixlabel->setPixmap( QPixmap(locate("data", "kcontrol/pics/energybig.png")));
	
	topLayout->addLayout(h1Layout);
	h1Layout->addWidget( cbEnable);
	h1Layout->addStretch( 10 );
	h1Layout->addWidget( pixlabel);

	if (!hasDPMS) {
	  QLabel *msg = new QLabel(
	    i18n("Your display has NO power saving features!"), this );
	  topLayout->addWidget(msg);
	}

	standbySlider = new KIntNumInput(i18n("&Standby after:"), 0, 120, 10, standby, i18n("min"), 10, true, this);
	standbySlider->setSteps( 10, 30 );
	standbySlider->setEnabled(hasDPMS);
    standbySlider->setSpecialValueText(i18n("Disabled"));
	connect(standbySlider, SIGNAL(valueChanged(int)), SLOT(slotChangeStandby()) );

	suspendSlider = new KIntNumInput(i18n("&Suspend after:"), 0, 120, 10, suspend, i18n("min"), 10, true, this);
	suspendSlider->setSteps( 10, 30 );
	suspendSlider->setEnabled(hasDPMS);
    suspendSlider->setSpecialValueText(i18n("Disabled"));
	connect(suspendSlider, SIGNAL(valueChanged(int)), SLOT(slotChangeSuspend()) );

	offSlider = new KIntNumInput(i18n("&Power Off after:"), 0, 120, 10, off, i18n("min"), 10, true, this);
	offSlider->setSteps( 10, 30 );
	offSlider->setEnabled(hasDPMS);
    offSlider->setSpecialValueText(i18n("Disabled"));
	connect(offSlider, SIGNAL(valueChanged(int)), SLOT(slotChangeOff()) );
		
	QBoxLayout *h2Layout= new QHBoxLayout();
	QBoxLayout *h3Layout= new QHBoxLayout();
	QBoxLayout *h4Layout= new QHBoxLayout();

	topLayout->addLayout(h2Layout);
	h2Layout->addStretch(10);
	topLayout->addWidget( standbySlider);
	topLayout->addSpacing(14);

	topLayout->addLayout(h3Layout);
	h3Layout->addStretch(10);
	topLayout->addWidget( suspendSlider);
	topLayout->addSpacing(14);

	topLayout->addLayout(h4Layout);
	h4Layout->addStretch(10);
	topLayout->addWidget( offSlider);

	topLayout->addStretch(10);
	topLayout->activate();
}


void KEnergy::slotChangeEnable()
{
	enabled= cbEnable->isChecked();
	changed=true;
}


void KEnergy::slotChangeStandby()
{
	standby= standbySlider->value();
	if (standby > suspend) {
	  suspendSlider->setValue(standby);
	  slotChangeSuspend();
	}
	changed=true;
}


void KEnergy::slotChangeSuspend()
{
	suspend= suspendSlider->value();
	if (suspend > off) {
	  offSlider->setValue(suspend);
	  slotChangeOff();
	} else if (suspend < standby) {
	  standbySlider->setValue(suspend);
	  slotChangeStandby();
	}
	changed=true;
}


void KEnergy::slotChangeOff()
{
	off= offSlider->value();
	if (off < suspend) {
	  suspendSlider->setValue(off);
	  slotChangeSuspend();
	}
	changed=true;
}


KEnergy::~KEnergy()
{
}


void KEnergy::readSettings( int )
{		
	QString str;

	KConfig *config= kapp->getConfig();
	config->setGroup( "DisplayEnergy" );

	str = config->readEntry( "displayEnergySaving", "off" );
	enabled= (str == "on");

	str = config->readEntry( "displayStandby", QString::number(DFLT_STANDBY));
	standby = str.toInt();

	str = config->readEntry( "displaySuspend", QString::number(DFLT_SUSPEND));
	suspend = str.toInt();

	str = config->readEntry( "displayPowerOff", QString::number(DFLT_OFF));
	off = str.toInt();
}


void KEnergy::setDefaults()
{
	cbEnable->setChecked( false );
	standbySlider->setValue( DFLT_STANDBY );
	suspendSlider->setValue( DFLT_SUSPEND );
	offSlider->setValue( DFLT_OFF );

	slotChangeEnable();
	slotChangeStandby();
	slotChangeSuspend();
	slotChangeOff();
}


void KEnergy::defaultSettings()
{
	setDefaults();
}


void KEnergy::writeSettings()
{
	if ( !changed )
		return;
		
	KConfig *config = kapp->getConfig();
	config->setGroup( "DisplayEnergy" );

	QString str;

	config->writeEntry( "displayEnergySaving",
		enabled ? "on" : "off");

	str.setNum(standby);
	config->writeEntry("displayStandby", str);

	str.setNum(suspend);
	config->writeEntry("displaySuspend", str);

	str.setNum(off);
	config->writeEntry("displayPowerOff", str);

	config->sync();
	
	QApplication::syncX();
}


void KEnergy::apply( bool force )
{
	if ( !changed && !force)
		return;
	
#if 1
	int (*defaultHandler)(Display *, XErrorEvent *);

	defaultHandler=XSetErrorHandler(dropError);

	if (hasDPMS) {
#if HAVE_DPMS
        if (enabled) {
            DPMSEnable(kde_display);
            DPMSSetTimeouts(kde_display,
                            60 * standby, 60 * suspend, 60 * off);
        } else {
            DPMSDisable(kde_display);
        }
#endif
	} else
        warning("Server has no DPMS extension");
	
	XFlush(kde_display);
	XSetErrorHandler(defaultHandler);
#else
	// One might use this instead of native X11 code...
	char str[100];
	sprintf(str, "/usr/bin/X11/xset dpms %d %d %d",
		enabled ? 60*standby : 0,
		enabled ? 60*suspend : 0,
		enabled ? 60*off: 0
	);
	system(str);
#endif
	
	changed=false;
}


void KEnergy::applySettings()
{
	writeSettings();
	apply( true );
}
