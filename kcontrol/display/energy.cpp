/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcontrol.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 *
 * Based on kcontrol1 energy.cpp, Copyright (c) 1999 Tom Vijlbrief
 */


/*
 * KDE Energy setup module.
 */

#include <config.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qglobal.h>
#include <qgroupbox.h>

#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <klocale.h>
#include <kcmodule.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "energy.h"


#if HAVE_DPMS
#include <X11/extensions/dpms.h>

extern "C" {
    int DPMSQueryExtension(Display *, int *, int *);
    void DPMSEnable(Display *);
    void DPMSDisable(Display *);
    void DPMSSetTimeouts(Display *, int, int, int);
}
#endif


static const int DFLT_STANDBY	= 0;
static const int DFLT_SUSPEND	= 30;
static const int DFLT_OFF	= 60;



/**** DLL Interface ****/

extern "C" {

    KCModule *create_energy(QWidget *parent, char *name) {
	return new KEnergy(parent, name);
    }

    void init_energy() {
	KConfig *cfg = new KConfig("kcmdisplayrc");
	cfg->setGroup("DisplayEnergy");

	bool enabled = cfg->readBoolEntry("displayEnergySaving", false);
	int standby = cfg->readNumEntry("displayStandby", DFLT_STANDBY);
	int suspend = cfg->readNumEntry("displaySuspend", DFLT_SUSPEND);
	int off = cfg->readNumEntry("displayPowerOff", DFLT_OFF);

	delete cfg;

	KEnergy::applySettings(enabled, standby, suspend, off);
    }
}



/**** KEnergy ****/

KEnergy::KEnergy(QWidget *parent, const char *name)
	: KCModule(parent, name)
{
    m_bChanged = false;
    m_bEnabled = false;
    m_Standby = DFLT_STANDBY;
    m_Suspend = DFLT_SUSPEND;
    m_Off = DFLT_OFF;
    m_bDPMS = false;

#if HAVE_DPMS
    int dummy;
    m_bDPMS = DPMSQueryExtension(qt_xdisplay(), &dummy, &dummy);
#endif

    QVBoxLayout *top = new QVBoxLayout(this, 10, 10);
    QHBoxLayout *hbox = new QHBoxLayout();
    top->addLayout(hbox);

    QLabel *lbl;
    if (m_bDPMS) {
	m_pCBEnable= new QCheckBox(i18n("&Enable Display Energy Saving" ), this);
	connect(m_pCBEnable, SIGNAL(toggled(bool)), SLOT(slotChangeEnable(bool)));
	hbox->addWidget(m_pCBEnable);
    } else {
	lbl = new QLabel(i18n("Your display has NO power saving features!"), this);
	hbox->addWidget(lbl);
    }

    lbl= new QLabel(this);
    lbl->setPixmap(QPixmap(locate("data", "kcontrol/pics/energybig.png")));
    hbox->addStretch();
    hbox->addWidget(lbl);
    
    // Sliders
    m_pStandbySlider = new KIntNumInput(i18n("&Standby after:"), 0, 120, 10, 
	    m_Standby, i18n("min"), 10, true, this);
    m_pStandbySlider->setSpecialValueText(i18n("Disabled"));
    connect(m_pStandbySlider, SIGNAL(valueChanged(int)), SLOT(slotChangeStandby(int)));
    top->addWidget(m_pStandbySlider);

    m_pSuspendSlider = new KIntNumInput(i18n("S&uspend after:"), 0, 120, 10, 
	    m_Suspend, i18n("min"), 10, true, this);
    m_pSuspendSlider->setSpecialValueText(i18n("Disabled"));
    connect(m_pSuspendSlider, SIGNAL(valueChanged(int)), SLOT(slotChangeSuspend(int)));
    top->addWidget(m_pSuspendSlider);

    m_pOffSlider = new KIntNumInput(i18n("&Power Off after:"), 0, 120, 10, 
	    m_Off, i18n("min"), 10, true, this);
    m_pOffSlider->setSpecialValueText(i18n("Disabled"));
    connect(m_pOffSlider, SIGNAL(valueChanged(int)), SLOT(slotChangeOff(int)));
    top->addWidget(m_pOffSlider);

    top->addStretch();
		
    m_pConfig = new KConfig("kcmdisplayrc");
    m_pConfig->setGroup("DisplayEnergy");

    load();
    setButtons(buttons());
}


KEnergy::~KEnergy()
{
    delete m_pConfig;
}


int KEnergy::buttons()
{
    if (m_bDPMS)
	return KCModule::Help | KCModule::Default | KCModule::Reset |
	       KCModule::Cancel | KCModule::Ok; 
    else
	return KCModule::Help | KCModule::Ok;
}


void KEnergy::load()
{
    readSettings();
    showSettings();

    emit changed(false);
}


void KEnergy::save()
{
    writeSettings();
    applySettings(m_bEnabled, m_Standby, m_Suspend, m_Off);

    emit changed(false);
}


void KEnergy::defaults()
{
    m_bEnabled = false;
    m_Standby = DFLT_STANDBY;
    m_Suspend = DFLT_SUSPEND;
    m_Off = DFLT_OFF;

    showSettings();
    emit changed(true);
}


void KEnergy::readSettings()
{		
    m_bEnabled = m_pConfig->readBoolEntry("displayEnergySaving", false);
    m_Standby = m_pConfig->readNumEntry("displayStandby", DFLT_STANDBY);
    m_Suspend = m_pConfig->readNumEntry("displaySuspend", DFLT_SUSPEND);
    m_Off = m_pConfig->readNumEntry("displayPowerOff", DFLT_OFF);

    m_bChanged = false;
}


void KEnergy::writeSettings()
{
    if (!m_bChanged)
	 return;
		
    m_pConfig->writeEntry( "displayEnergySaving", m_bEnabled);
    m_pConfig->writeEntry("displayStandby", m_Standby);
    m_pConfig->writeEntry("displaySuspend", m_Suspend);
    m_pConfig->writeEntry("displayPowerOff", m_Off);

    m_pConfig->sync();
    m_bChanged = false;
}


void KEnergy::showSettings()
{
    if (m_bDPMS)
	m_pCBEnable->setChecked(m_bEnabled);

    m_pStandbySlider->setEnabled(m_bEnabled);
    m_pStandbySlider->setValue(m_Standby);
    m_pSuspendSlider->setEnabled(m_bEnabled);
    m_pSuspendSlider->setValue(m_Suspend);
    m_pOffSlider->setEnabled(m_bEnabled);
    m_pOffSlider->setValue(m_Off);
}


int dropError(Display *, XErrorEvent *)
{
    return 0;
}

/* static */
void KEnergy::applySettings(bool enable, int standby, int suspend, int off)
{
#if HAVE_DPMS
    int (*defaultHandler)(Display *, XErrorEvent *);
    defaultHandler = XSetErrorHandler(dropError);

    Display *dpy = qt_xdisplay();

    int dummy;
    bool hasDPMS = DPMSQueryExtension(dpy, &dummy, &dummy);
    if (hasDPMS) {
	if (enable) {
            DPMSEnable(dpy);
            DPMSSetTimeouts(dpy, 60*standby, 60*suspend, 60*off);
        } else 
            DPMSDisable(dpy);
    } else
	warning("Server has no DPMS extension");
	
    XFlush(dpy);
    XSetErrorHandler(defaultHandler);
#else
    /* keep gcc silent */
    if (enable | standby | suspend | off)
	/* nothing */ ;
#endif
}


void KEnergy::slotChangeEnable(bool ena)
{
    m_bEnabled = ena;
    m_bChanged = true;

    m_pStandbySlider->setEnabled(ena);
    m_pSuspendSlider->setEnabled(ena);
    m_pOffSlider->setEnabled(ena);
}


void KEnergy::slotChangeStandby(int value)
{
    m_Standby = value;
    if (m_Standby > m_Suspend)
	m_pSuspendSlider->setValue(m_Standby);

    m_bChanged = true;
    emit changed(true);
}


void KEnergy::slotChangeSuspend(int value)
{
    m_Suspend = value;
    if (m_Suspend > m_Off)
	m_pOffSlider->setValue(m_Suspend);
    if (m_Suspend < m_Standby)
	m_pStandbySlider->setValue(m_Suspend);

    m_bChanged = true;
    emit changed(true);
}


void KEnergy::slotChangeOff(int value)
{
    m_Off = value;
    if (m_Off < m_Suspend)
	m_pSuspendSlider->setValue(m_Off);

    m_bChanged = true;
    emit changed(true);
}

#include "energy.moc"
