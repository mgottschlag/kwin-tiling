//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//
// 1998/04/19 Layout management added by Mario Weilguni <mweilguni@kde.org>
// 2001/03/04 Converted to use libkscreensaver by Martin R. Jones

#include <stdlib.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QFrame>
#include <QGridLayout>
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcolordialog.h>
#include <kbuttonbox.h>
#include <kcolorbutton.h>
#include <kglobal.h>
#include "blankscrn.h"
#include "blankscrn.moc"

// libkscreensaver interface
extern "C"
{
    KDE_EXPORT const char *kss_applicationName = "kblankscrn.kss";
    KDE_EXPORT const char *kss_description = I18N_NOOP( "KBlankScreen" );
    KDE_EXPORT const char *kss_version = "2.2.0";

    KDE_EXPORT KScreenSaver* kss_create( WId id )
    {
        return new KBlankSaver( id );
    }

    KDE_EXPORT QDialog* kss_setup()
    {
        return new KBlankSetup();
    }
}

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
KBlankSetup::KBlankSetup( QWidget *parent, const char *name )
	: KDialogBase( parent, name, true, i18n( "Setup Blank Screen Saver" ),
		Ok|Cancel, Ok, true )
{
	readSettings();

	QFrame *main = makeMainWidget();
	QGridLayout *grid = new QGridLayout(main, 4, 2, 0, spacingHint() );

	QLabel *label = new QLabel( i18n("Color:"), main );
	grid->addWidget(label, 0, 0);

	KColorButton *colorPush = new KColorButton( color, main );
	colorPush->setMinimumWidth(80);
	connect( colorPush, SIGNAL( changed(const QColor &) ),
		SLOT( slotColor(const QColor &) ) );
	grid->addWidget(colorPush, 1, 0);

	preview = new QWidget( main );
	preview->setFixedSize( 220, 165 );
	preview->setBackgroundColor( Qt::black );
	preview->show();    // otherwise saver does not get correct size
	saver = new KBlankSaver( preview->winId() );
	grid->addMultiCellWidget(preview, 0, 2, 1, 1);

	grid->setRowStretch( 2, 10 );
	grid->setRowStretch( 3, 20 );

	setMinimumSize( sizeHint() );
}

// read settings from config file
void KBlankSetup::readSettings()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "Settings" );

	QColor aux = Qt::black;
	color = config->readEntry( "Color", aux );
}

void KBlankSetup::slotColor( const QColor &col )
{
    color = col;
    saver->setColor( color );
}

// Ok pressed - save settings and exit
void KBlankSetup::slotOk()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "Settings" );
	config->writeEntry( "Color", color );
	config->sync();

	accept();
}

//-----------------------------------------------------------------------------


KBlankSaver::KBlankSaver( WId id ) : KScreenSaver( id )
{
	readSettings();
	blank();
}

KBlankSaver::~KBlankSaver()
{
}

// set the color
void KBlankSaver::setColor( const QColor &col )
{
	color = col;
	blank();
}

// read configuration settings from config file
void KBlankSaver::readSettings()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "Settings" );

	QColor aux = Qt::black;
	color = config->readEntry( "Color", aux );
}

void KBlankSaver::blank()
{
    setBackgroundColor( color );
    erase();
}

