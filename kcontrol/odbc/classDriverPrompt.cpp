#include <qpixmap.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbt.h>

#include "classDriverPrompt.h"
#include "classDriverPrompt.moc"


classDriverPrompt::classDriverPrompt( QWidget* parent, const char* name, bool modal )
	: QDialog( parent, name, modal )
{
	pDrivers = new classDrivers( this, "Drivers" );
	pDrivers->setGeometry( 1, 25, 390, 300 );
	pDrivers->setMinimumSize( 0, 0 );
	pDrivers->setMaximumSize( 32767, 32767 );
	pDrivers->setFocusPolicy( QWidget::NoFocus );
	pDrivers->setBackgroundMode( QWidget::PaletteBackground );
	pDrivers->setFontPropagation( QWidget::NoChildren );
	pDrivers->setPalettePropagation( QWidget::NoChildren );
    pDrivers->show();

	QLabel* qtarch_Label_10;
	qtarch_Label_10 = new QLabel( this, "Label_10" );
	qtarch_Label_10->setGeometry( 10, 10, 400, 20 );
	qtarch_Label_10->setMinimumSize( 0, 0 );
	qtarch_Label_10->setMaximumSize( 32767, 32767 );
	qtarch_Label_10->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_10->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_10->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_10->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_10->setText( "Select the DRIVER to use or Add a new one..." );
	qtarch_Label_10->setAlignment( 289 );
	qtarch_Label_10->setMargin( -1 );

	QFrame *fra	= new QFrame( this );
	fra->setGeometry( 400, 10, 2, 300 );
	fra->setFrameStyle( 33 );


	QPushButton* qtarch_pbOk;
	qtarch_pbOk = new QPushButton( this, "pbOk" );
	qtarch_pbOk->setGeometry( 420, 230, 100, 30 );
	qtarch_pbOk->setMinimumSize( 0, 0 );
	qtarch_pbOk->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbOk, SIGNAL(clicked()), SLOT(pbOk_Clicked()) );
	qtarch_pbOk->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbOk->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbOk->setFontPropagation( QWidget::NoChildren );
	qtarch_pbOk->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbOk->setText( "&Ok" );
	qtarch_pbOk->setAutoRepeat( FALSE );
	qtarch_pbOk->setAutoResize( FALSE );

	QPushButton* qtarch_pbCancel;
	qtarch_pbCancel = new QPushButton( this, "pbCancel" );
	qtarch_pbCancel->setGeometry( 420, 270, 100, 30 );
	qtarch_pbCancel->setMinimumSize( 0, 0 );
	qtarch_pbCancel->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbCancel, SIGNAL(clicked()), SLOT(pbCancel_Clicked()) );
	qtarch_pbCancel->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbCancel->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbCancel->setFontPropagation( QWidget::NoChildren );
	qtarch_pbCancel->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbCancel->setText( "&Cancel" );
	qtarch_pbCancel->setAutoRepeat( FALSE );
	qtarch_pbCancel->setAutoResize( FALSE );

	resize( 530,335 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );

	setCaption( "Adding a new data source" );
	qsDriverName	= "";
	qsDescription		= "";
	qsDriver		= "";
	qsSetup			= "";
}


classDriverPrompt::~classDriverPrompt()
{
}

/********************************************
 * protected slots
 ********************************************/

void classDriverPrompt::pbCancel_Clicked()
{
	reject();
}

void classDriverPrompt::pbOk_Clicked()
{
    QListView		*pListView;
    QListViewItem	*pListViewItem;

	pListView		= pDrivers->getListView();
	pListViewItem	= pListView->currentItem();

	if ( pListViewItem )
	{
		qsDriverName	= pListViewItem->text( 0 );
		qsDescription		= pListViewItem->text( 1 );
		qsDriver		= pListViewItem->text( 2 );
		qsSetup			= pListViewItem->text( 3 );
		if ( qsDriverName == "" )
			QMessageBox::information(	this, "ODBC Config",  "Please select a listing which contains a Driver file name" );
		else
			accept();
	}
	else
		QMessageBox::information(	this, "ODBC Config",  "Please select a Driver from the list or click Cancel" );
}



