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
#include <kbuttonbox.h>
#include <kcolorbutton.h>
#include <kglobal.h>
#include "blankscrn.h"
#include <kaboutdata.h>
#include "blankscrn.moc"

// libkscreensaver interface
class KBlankSaverInterface : public KScreenSaverInterface
{
public:
    virtual KAboutData* aboutData()
    {
        return new KAboutData( "kblankscrn.kss", I18N_NOOP( "KBlankScreen" ),
                               "4.0.0", I18N_NOOP( "Blank Screen Saver" ) );
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
  enableButtonSeparator( true );

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
	connect( colorPush, SIGNAL( changed(const QColor &) ),
		SLOT( slotColor(const QColor &) ) );
	grid->addWidget(colorPush, 1, 0);

	preview = new QWidget( main );
	preview->setFixedSize( 220, 165 );
	QPalette palette;
	palette.setColor( preview->backgroundRole(), Qt::black );
	preview->setPalette(palette);
	preview->show();    // otherwise saver does not get correct size
	saver = new KBlankSaver( preview->winId() );
	grid->addWidget(preview, 0, 1, 3, 1);

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
	QPalette palette;
	palette.setColor( backgroundRole(), color );
	setPalette(palette);
	update();
}

