//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//
// 1998/04/19 Layout management added by Mario Weilguni <mweilguni@kde.org>
// 2001/03/04 Converted to use libkscreensaver by Martin R. Jones
// 2006/03/12 Ported to KScreenSaverInterface by David Faure

#include <stdlib.h>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QGridLayout>
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcolordialog.h>
#include <kcolorbutton.h>
#include <kglobal.h>
#include "blankscrn.h"
#include <kaboutdata.h>
#include <kdeversion.h>
#include "blankscrn.moc"

// libkscreensaver interface
class KBlankSaverInterface : public KScreenSaverInterface
{
public:
    virtual KAboutData* aboutData()
    {
        return new KAboutData( "kblankscrn.kss", "kblankscrn", ki18n( "KBlankScreen" ),
                               KDE_VERSION_STRING, ki18n( "Blank Screen Saver" ) );
    }

    virtual KScreenSaver* create( WId id )
    {
        return new KBlankSaver( id );
    }

    virtual QDialog* setup()
    {
        return new KBlankSetup();
    }
};

int main( int argc, char *argv[] )
{
    KBlankSaverInterface kss;
    return kScreenSaverMain( argc, argv, kss );
}

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
KBlankSetup::KBlankSetup( QWidget *parent, const char *name )
	: KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "Setup Blank Screen Saver" ) );
  setButtons( Ok | Cancel );

	readSettings();

	QFrame *main = new QFrame( this );
  setMainWidget( main );
	QGridLayout *grid = new QGridLayout(main);
	grid->setSpacing(spacingHint());
	grid->setMargin(0);

	QLabel *label = new QLabel( i18n("Color:"), main );
	grid->addWidget(label, 0, 0);

	KColorButton *colorPush = new KColorButton( color, main );
	colorPush->setMinimumWidth(80);
	connect( colorPush, SIGNAL(changed(QColor)),
		SLOT(slotColor(QColor)) );
	grid->addWidget(colorPush, 1, 0);

	preview = new QWidget( main );
	preview->setFixedSize( 220, 165 );
	QPalette palette;
	palette.setColor( preview->backgroundRole(), Qt::black );
	preview->setPalette(palette);
	preview->setAutoFillBackground(true);
	preview->show();    // otherwise saver does not get correct size
	saver = new KBlankSaver( preview->winId() );
	grid->addWidget(preview, 0, 1, 3, 1);

	grid->setRowStretch( 2, 10 );
	grid->setRowStretch( 3, 20 );

	setMinimumSize( sizeHint() );
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
}

// read settings from config file
void KBlankSetup::readSettings()
{
	KConfigGroup config(KGlobal::config(), "Settings");

	QColor aux = Qt::black;
	color = config.readEntry( "Color", aux );
}

void KBlankSetup::slotColor( const QColor &col )
{
    color = col;
    saver->setColor( color );
}

// Ok pressed - save settings and exit
void KBlankSetup::slotOk()
{
	KConfigGroup config(KGlobal::config(), "Settings");
 	config.writeEntry( "Color", color );
  config.sync();

 	accept();
}

//-----------------------------------------------------------------------------


KBlankSaver::KBlankSaver( WId id ) : KScreenSaver( id )
{
	readSettings();
	blank();
	setAutoFillBackground(true);
	show();
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
	KConfigGroup config(KGlobal::config(), "Settings");

	QColor aux = Qt::black;
	color = config.readEntry( "Color", aux );
}

void KBlankSaver::blank()
{
	QPalette palette;
	palette.setColor( backgroundRole(), color );
	setPalette(palette);
}

