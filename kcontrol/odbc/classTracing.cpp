#include "classTracing.h"
#include "classTracing.moc"

#include "pics/trace.xpm"

classTracing::classTracing( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	QBoxLayout	*playoutTop		= new QVBoxLayout( this, 20 );
	QGridLayout	*playoutMain	= new QGridLayout( playoutTop, 0, 0, 10 );

	// main
	plabelTracing	= new QLabel( "Tracing Enabled", this );
	pTracing 		= new QCheckBox( this );
	QLabel *pLabel	= new QLabel( "Trace File", this );
	pTraceFile		= new classFileSelector( this );

	playoutMain->addWidget( plabelTracing, 0, 0 );
    playoutMain->addWidget( pTracing, 0, 1 );
    playoutMain->addWidget( pLabel, 1, 0 );
    playoutMain->addWidget( pTraceFile, 1, 1 );

//	pVBox->setGeometry( 10, 10, 380, 290 );


	// Apply
	QBoxLayout	*playoutApply	= new QHBoxLayout( playoutTop );
	pApply = new QPushButton( "&Apply", this );
	playoutApply->addStretch( 10 );
	playoutApply->addWidget( pApply, 5 );

	// helpt text
	QBoxLayout	*playoutHelp = new QHBoxLayout( playoutTop );

	QFrame* qtarch_Frame_7;
	qtarch_Frame_7 = new QFrame( this, "Frame_7" );
	qtarch_Frame_7->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_7->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_7->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_7->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_7->setFrameStyle( QFrame::Box | QFrame::Raised );

	playoutHelp->addWidget( qtarch_Frame_7, 5 );

	QLabel* qtarch_Label_2;
	qtarch_Label_2 = new QLabel( qtarch_Frame_7, "Label_2" );
	qtarch_Label_2->setGeometry( 20, 20, 32, 32 );
	qtarch_Label_2->setBackgroundPixmap( QPixmap( trace_xpm ) );
	qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_2->setText( "" );
	qtarch_Label_2->setAlignment( 289 );
	qtarch_Label_2->setMargin( -1 );

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( qtarch_Frame_7, "Label_1" );
	qtarch_Label_1->setGeometry( 70, 10, 280, 80 );
	qtarch_Label_1->setText( "Tracing allows you to create logs of the calls to ODBC drivers. Great for support persons or to aid you in debugging applications.\nYou must be 'root' to set." );
	qtarch_Label_1->setAlignment( AlignLeft | WordBreak );

	playoutTop->activate();

	resize( 400,300 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );


	// init values        
	char szTracing[ 64 ];
	char szTracingFile[ 64 ];

	SQLGetPrivateProfileString( "ODBC", "Trace", "No", szTracing, sizeof( szTracing ), "odbcinst.ini" );
	if ( szTracing[ 0 ] == '1' || toupper( szTracing[ 0 ] ) == 'Y' || toupper( szTracing[ 0 ] ) == 'O' )
		pTracing->setChecked( true );
		
	SQLGetPrivateProfileString( "ODBC", "Trace File", "/tmp/sql.log", szTracingFile, sizeof( szTracingFile ), "odbcinst.ini" );
	pTraceFile->pLineEdit->setText( szTracingFile );

	connect( pApply, SIGNAL(clicked()), SLOT(apply()) );
}

classTracing::~classTracing()
{
}


// slots

void classTracing::apply()
{
	char szTracing[10];
	char szTracingFile[FILENAME_MAX+1];

	if ( pTracing->isChecked() )
		strcpy( szTracing, "Yes" );
	else
		strcpy( szTracing, "No" );

	if ( !SQLWritePrivateProfileString( "ODBC", "Trace", szTracing, "odbcinst.ini" ) )
	{
		KMessageBox::information( this, "Could not apply. Ensure that you are operating as 'root' user." );
		return;
	}
	else		
	{
		strncpy( szTracingFile, pTraceFile->pLineEdit->text().ascii(), FILENAME_MAX );
		SQLWritePrivateProfileString( "ODBC", "Trace File", szTracingFile, "odbcinst.ini" );
	}


	if ( pTracing->isChecked() )
		KMessageBox::information( this, "Tracing is turned on. Tracing uses up disk space as all calls are logged. Ensure that you turn it off as soon as possible." );
}

