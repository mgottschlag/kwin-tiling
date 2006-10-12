/*
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdm-font.h"

#include <kdialog.h>
#include <kfontrequester.h>
#include <klocale.h>
#include <ksimpleconfig.h>

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

extern KSimpleConfig *config;

KDMFontWidget::KDMFontWidget( QWidget *parent )
	: QWidget( parent )
{
	QGridLayout *ml = new QGridLayout( this );
	ml->setSpacing( KDialog::spacingHint() );
	ml->setMargin( KDialog::marginHint() );
	QLabel *label = new QLabel( i18n("&General:"),this );
	stdFontChooser = new KFontRequester( this );
	label->setBuddy( stdFontChooser );
	stdFontChooser->setWhatsThis( i18n("This changes the font which is used for all the text in the login manager except for the greeting and failure messages.") );
	connect( stdFontChooser, SIGNAL(fontSelected( const QFont& )), SIGNAL(changed()) );
	ml->addWidget( label, 1, 0 );
	ml->addWidget( stdFontChooser, 1, 1 );

	label = new QLabel( i18n("&Failures:"),this );
	failFontChooser = new KFontRequester( this );
	label->setBuddy( failFontChooser );
	failFontChooser->setWhatsThis( i18n("This changes the font which is used for failure messages in the login manager.") );
	connect( failFontChooser, SIGNAL(fontSelected( const QFont& )), SIGNAL(changed()) );
	ml->addWidget( label, 2, 0 );
	ml->addWidget( failFontChooser, 2, 1 );

	label = new QLabel( i18n("Gree&ting:"),this );
	greetingFontChooser = new KFontRequester( this );
	label->setBuddy( greetingFontChooser );
	greetingFontChooser->setWhatsThis( i18n("This changes the font which is used for the login manager's greeting.") );
	connect( greetingFontChooser, SIGNAL(fontSelected( const QFont& )), SIGNAL(changed()) );
	ml->addWidget( label, 3, 0 );
	ml->addWidget( greetingFontChooser, 3, 1 );

	aacb = new QCheckBox( i18n("Use anti-aliasing for fonts"), this );
	aacb->setWhatsThis( i18n("If you check this box and your X-Server has the Xft extension, "
	                         "fonts will be antialiased (smoothed) in the login dialog.") );
	connect( aacb, SIGNAL(toggled( bool )), SIGNAL(changed()) );
	ml->addWidget( aacb, 4, 0, 1, 2 );
	ml->setRowStretch( 5, 10 );
}

void KDMFontWidget::makeReadOnly()
{
	stdFontChooser->button()->setEnabled( false );
	failFontChooser->button()->setEnabled( false );
	greetingFontChooser->button()->setEnabled( false );
	aacb->setEnabled( false );
}

void KDMFontWidget::set_def()
{
	stdFontChooser->setFont( QFont( "Sans Serif", 10 ) );
	failFontChooser->setFont( QFont( "Sans Serif", 10, QFont::Bold ) );
	greetingFontChooser->setFont( QFont( "Serif", 20 ) );
}

void KDMFontWidget::save()
{
	config->setGroup( "X-*-Greeter" );

	// write font
	config->writeEntry( "StdFont", stdFontChooser->font() );
	config->writeEntry( "GreetFont", greetingFontChooser->font() );
	config->writeEntry( "FailFont", failFontChooser->font() );
	config->writeEntry( "AntiAliasing", aacb->isChecked() );
}


void KDMFontWidget::load()
{
	set_def();

	config->setGroup( "X-*-Greeter" );

	// Read the fonts
	QFont font = stdFontChooser->font();
	stdFontChooser->setFont( config->readEntry( "StdFont", font ) );
	font = failFontChooser->font();
	failFontChooser->setFont( config->readEntry( "FailFont", font ) );
	font = greetingFontChooser->font();
	greetingFontChooser->setFont( config->readEntry( "GreetFont", font ) );

	aacb->setChecked( config->readEntry( "AntiAliasing", false ) );
}


void KDMFontWidget::defaults()
{
	set_def();
	aacb->setChecked( false );
}

#include "kdm-font.moc"
