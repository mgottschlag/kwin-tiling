//
// KDE Energy saver setup module
//
// Written by: Tom Vijlbrief 1999
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qfiledlg.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qlayout.h>
#include <qlcdnum.h>
#include <kapp.h>
#include <kcharsets.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kwm.h>
#include <kcolordlg.h>
#include <kiconloader.h>
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

KEnergy::KEnergy( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	changed= false;
	enabled= false;

	standby= DFLT_STANDBY;
	suspend= DFLT_SUSPEND;
	off= DFLT_OFF;

	debug("KEnergy::KEnergy");
	
	readSettings();

	kde_display = x11Display();
    hasDPMS = false;

#if HAVE_DPMS
    int dummy;
    hasDPMS= DPMSQueryExtension(kde_display, &dummy, &dummy);
#endif

	apply(true);

	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;

	
	setName( i18n("Energy") );
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	QBoxLayout *h1Layout= new QHBoxLayout();

	cbEnable= new QCheckBox( i18n( "&Enable Display Energy Saving" ),
				 this );
	cbEnable->setMinimumSize(cbEnable->sizeHint());
	cbEnable->setChecked(enabled);
	cbEnable->setEnabled(hasDPMS);
	connect(cbEnable, SIGNAL(clicked()), SLOT(slotChangeEnable()));

	KIconLoader iconLoader;
	QPixmap p = iconLoader.loadIcon("energybig.xpm");
	QLabel *pixlabel= new QLabel(this);
	pixlabel->setPixmap(p);
	pixlabel->setMinimumSize(pixlabel->sizeHint());	
	
	topLayout->addLayout(h1Layout);
	h1Layout->addWidget( cbEnable);
	h1Layout->addStretch( 10 );
	h1Layout->addWidget( pixlabel);

	if (!hasDPMS) {
	  QLabel *msg = new QLabel(
	    i18n("Your display has NO power saving features!"), this );
	  msg->setMinimumSize(msg->sizeHint());
	  topLayout->addWidget(msg);
	}

	standbySlider = new QSlider( QSlider::Horizontal, this);
	standbySlider->setRange( 0, 120 );
	standbySlider->setSteps( 10, 30 );
	standbySlider->setTickmarks( QSlider::Below );
	standbySlider->setFixedHeight( standbySlider->sizeHint().height() );
	standbySlider->setValue( standby );
	standbySlider->setEnabled(hasDPMS);
	connect(standbySlider, SIGNAL(valueChanged(int)),
		SLOT(slotChangeStandby()) );

	suspendSlider = new QSlider( QSlider::Horizontal, this);
	suspendSlider->setRange( 0, 120 );
	suspendSlider->setSteps( 10, 30 );
	suspendSlider->setTickmarks( QSlider::Below );
	suspendSlider->setFixedHeight( suspendSlider->sizeHint().height() );
	suspendSlider->setValue( suspend );
	suspendSlider->setEnabled(hasDPMS);
	connect(suspendSlider, SIGNAL(valueChanged(int)),
		SLOT(slotChangeSuspend()) );

	offSlider = new QSlider( QSlider::Horizontal, this);
	offSlider->setRange( 0, 120 );
	offSlider->setSteps( 10, 30 );
	offSlider->setTickmarks( QSlider::Below );
	offSlider->setFixedHeight( offSlider->sizeHint().height() );
	offSlider->setValue( off );
	offSlider->setEnabled(hasDPMS);
	connect(offSlider, SIGNAL(valueChanged(int)),
		SLOT(slotChangeOff()) );
		
	QLabel *label0 = new QLabel( standbySlider,
	  i18n("&Standby after ... minutes, choose `0' to disable: "), this );
	label0->setMinimumSize(label0->sizeHint());

	QLabel *label1 = new QLabel( suspendSlider,
	  i18n("&Suspend after ... minutes, choose `0' to disable: "), this );
	label1->setMinimumSize(label1->sizeHint());

	QLabel *label2= new QLabel( offSlider,
	  i18n("&Power Off after ... minutes, choose `0' to disable: "), this );
	label2->setMinimumSize(label2->sizeHint());

	label0->setEnabled(hasDPMS);
	label1->setEnabled(hasDPMS);
	label2->setEnabled(hasDPMS);

	standbyLCD = new QLCDNumber(3, this);
	standbyLCD->setFrameStyle( QFrame::NoFrame );
	standbyLCD->setFixedHeight(40);
	standbyLCD->setMinimumSize(standbyLCD->sizeHint());
	standbyLCD->setEnabled(hasDPMS);
	standbyLCD->display( standby );

	suspendLCD = new QLCDNumber(3, this);
	suspendLCD->setFrameStyle( QFrame::NoFrame );
	suspendLCD->setFixedHeight(40);
	suspendLCD->setMinimumSize(suspendLCD->sizeHint());
	suspendLCD->setEnabled(hasDPMS);
	suspendLCD->display( suspend );

	offLCD = new QLCDNumber(3, this);
	offLCD->setFrameStyle( QFrame::NoFrame );
	offLCD->setFixedHeight(40);
	offLCD->setMinimumSize(offLCD->sizeHint());
	offLCD->setEnabled(hasDPMS);
	offLCD->display( off );

	QBoxLayout *h2Layout= new QHBoxLayout();
	QBoxLayout *h3Layout= new QHBoxLayout();
	QBoxLayout *h4Layout= new QHBoxLayout();

	topLayout->addLayout(h2Layout);
	h2Layout->addWidget( label0);
	h2Layout->addStretch(10);
	h2Layout->addWidget( standbyLCD);
	topLayout->addWidget( standbySlider);
	topLayout->addSpacing(14);

	topLayout->addLayout(h3Layout);
	h3Layout->addWidget( label1);
	h3Layout->addStretch(10);
	h3Layout->addWidget( suspendLCD);
	topLayout->addWidget( suspendSlider);
	topLayout->addSpacing(14);

	topLayout->addLayout(h4Layout);
	h4Layout->addWidget( label2);
	h4Layout->addStretch(10);
	h4Layout->addWidget( offLCD);
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
	standbyLCD->display(standby);
	if (standby > suspend) {
	  suspendSlider->setValue(standby);
	  slotChangeSuspend();
	}
	changed=true;
}


void KEnergy::slotChangeSuspend()
{
	suspend= suspendSlider->value();
	suspendLCD->display(suspend);
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
	offLCD->display(off);
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

	char def[100];

	sprintf(def, "%d", DFLT_STANDBY);
	str = config->readEntry( "displayStandby", def);
	standby = atoi( str );  

	sprintf(def, "%d", DFLT_SUSPEND);
	str = config->readEntry( "displaySuspend", def);
	suspend = atoi( str );  

	sprintf(def, "%d", DFLT_OFF);
	str = config->readEntry( "displayPowerOff", def);
	off = atoi( str );  
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


void KEnergy::slotApply()
{
	writeSettings();
	apply();
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
	  fprintf(stderr, "Server has no DPMS extension\n");
	
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


void KEnergy::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-8.html", "" );
}


void KEnergy::applySettings()
{
	writeSettings();
	apply( true );
}
