#include <qpixmap.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>

#include <kstddirs.h>

#include "classUserDSN.h"
#include "classUserDSN.moc"

classUserDSN::classUserDSN( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	pbAdd = new QPushButton( this, "pbAdd" );
	pbAdd->setGeometry( 290, 10, 100, 30 );
	pbAdd->setMinimumSize( 0, 0 );
	pbAdd->setMaximumSize( 32767, 32767 );
	pbAdd->setFocusPolicy( QWidget::TabFocus );
	pbAdd->setBackgroundMode( QWidget::PaletteBackground );
	pbAdd->setFontPropagation( QWidget::NoChildren );
	pbAdd->setPalettePropagation( QWidget::NoChildren );
	pbAdd->setText( "A&dd..." );
	pbAdd->setAutoRepeat( FALSE );
	pbAdd->setAutoResize( FALSE );

	pbRemove = new QPushButton( this, "pbRemove" );
	pbRemove->setGeometry( 290, 50, 100, 30 );
	pbRemove->setMinimumSize( 0, 0 );
	pbRemove->setMaximumSize( 32767, 32767 );
	pbRemove->setFocusPolicy( QWidget::TabFocus );
	pbRemove->setBackgroundMode( QWidget::PaletteBackground );
	pbRemove->setFontPropagation( QWidget::NoChildren );
	pbRemove->setPalettePropagation( QWidget::NoChildren );
	pbRemove->setText( "&Remove" );
	pbRemove->setAutoRepeat( FALSE );
	pbRemove->setAutoResize( FALSE );

	pbConfigure = new QPushButton( this, "pbConfigure" );
	pbConfigure->setGeometry( 290, 90, 100, 30 );
	pbConfigure->setMinimumSize( 0, 0 );
	pbConfigure->setMaximumSize( 32767, 32767 );
	pbConfigure->setFocusPolicy( QWidget::TabFocus );
	pbConfigure->setBackgroundMode( QWidget::PaletteBackground );
	pbConfigure->setFontPropagation( QWidget::NoChildren );
	pbConfigure->setPalettePropagation( QWidget::NoChildren );
	pbConfigure->setText( "&Configure..." );
	pbConfigure->setAutoRepeat( FALSE );
	pbConfigure->setAutoResize( FALSE );

	QFrame* qtarch_Frame_2;
	qtarch_Frame_2 = new QFrame( this, "Frame_2" );
	qtarch_Frame_2->setGeometry( 10, 210, 380, 80 );
	qtarch_Frame_2->setMinimumSize( 0, 0 );
	qtarch_Frame_2->setMaximumSize( 32767, 32767 );
	qtarch_Frame_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_2->setFrameStyle( 33 );

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 90, 220, 290, 60 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( "User data source configuration is stored in your home directory. This allows you to configure data access without having to be the system administrator." );
	qtarch_Label_1->setAlignment( 1313 );
	qtarch_Label_1->setMargin( -1 );

	QLabel* qtarch_Label_2;
	qtarch_Label_2 = new QLabel( this, "Label_2" );
	qtarch_Label_2->setGeometry( 30, 230, 32, 32 );
	qtarch_Label_2->setMinimumSize( 32, 32 );
	qtarch_Label_2->setMaximumSize( 32, 32 );
	qtarch_Label_2->setBackgroundPixmap( QPixmap( locate("icons", "person.png") ) );
	qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_2->setText( "" );
	qtarch_Label_2->setAlignment( 289 );
	qtarch_Label_2->setMargin( -1 );

	pDSNList = new classDSNList( this, "pDSNList" );
	pDSNList->setGeometry( 10, 10, 270, 190 );
	pDSNList->setMinimumSize( 0, 0 );
	pDSNList->setMaximumSize( 32767, 32767 );
	pDSNList->setFocusPolicy( QWidget::NoFocus );
	pDSNList->setBackgroundMode( QWidget::PaletteBackground );
	pDSNList->setFontPropagation( QWidget::NoChildren );
	pDSNList->setPalettePropagation( QWidget::NoChildren );

	resize( 400,300 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
	
	
	pDSNList->Load( ODBC_USER_DSN );

	connect( pbAdd, SIGNAL(clicked()), pDSNList, SLOT(Add()) );
	connect( pbRemove, SIGNAL(clicked()), pDSNList, SLOT(Delete()) );
	connect( pbConfigure, SIGNAL(clicked()), pDSNList, SLOT(Edit()) );
}


classUserDSN::~classUserDSN()
{
}

