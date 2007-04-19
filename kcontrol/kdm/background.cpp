/*
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * Modified 2000.07.14 by Brad Hughes <bhughes@trolltech.com>
 * Improve layout and consistency with KDesktop's background selection
 *
 * Based on old backgnd.cpp:
 *
 * Copyright (c)  Martin R. Jones 1996
 * Converted to a kcc module by Matthias Hoelzer 1997
 * Gradient backgrounds by Mark Donohoe 1997
 * Pattern backgrounds by Stephan Kulow 1998
 * Randomizing & dnd & new display modes by Matej Koss 1998
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <config.h>

#include "background.h"

#include "../background/bgdialog.h"

#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

#include <QCheckBox>
#include <QVBoxLayout>

extern KConfig *config;

KBackground::KBackground(QWidget *parent)
	: QWidget( parent )
{

	// Enabling checkbox
	m_pCBEnable = new QCheckBox( i18n("E&nable background"), this );
	m_pCBEnable->setWhatsThis(
		i18n("If this is checked, KDM will use the settings below for the background."
		     " If it is disabled, you have to look after the background yourself."
		     " This is done by running some program (possibly xsetroot) in the script"
		     " specified in the Setup= option in kdmrc (usually Xsetup).") );
	m_simpleConf = KSharedConfig::openConfig( config->group( "X-*-Greeter" ).readEntry( "BackgroundCfg", KDE_CONFDIR "/kdm/backgroundrc" ) );
	m_background = new BGDialog( this, m_simpleConf, false );

	connect( m_background, SIGNAL(changed( bool )), SIGNAL(changed()) );

	// Top layout
	QVBoxLayout *top = new QVBoxLayout( this );
	top->setMargin( KDialog::marginHint() );
	top->setSpacing( KDialog::spacingHint() );
	top->addWidget( m_pCBEnable );
	top->addWidget( m_background );
	top->addStretch();
	connect( m_pCBEnable, SIGNAL(toggled( bool )), SLOT(slotEnableChanged()) );
}

KBackground::~KBackground()
{
}

void KBackground::slotEnableChanged()
{
	bool en = m_pCBEnable->isChecked();
	m_background->setEnabled( en );
	emit changed();
}

void KBackground::makeReadOnly()
{
	m_pCBEnable->setEnabled( false );
	m_background->makeReadOnly();
}

void KBackground::load()
{
	m_pCBEnable->setChecked( config->group( "X-*-Greeter" ).readEntry( "UseBackground", true ) );
	m_background->load();
	slotEnableChanged();
}


void KBackground::save()
{
	config->group( "X-*-Greeter" ).writeEntry( "UseBackground", m_pCBEnable->isChecked() );
	m_background->save();
}


void KBackground::defaults()
{
	m_pCBEnable->setChecked( true );
	slotEnableChanged();
	m_background->defaults();
}

#include "background.moc"
