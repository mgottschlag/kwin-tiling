#include <qpixmap.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qframe.h>

#include "classAbout.h"
#include "classAbout.moc"

#include "info.xpm"

classAbout::classAbout( QWidget* parent, const char* name )
	: KConfigWidget( parent, name )
{
	QFrame* qtarch_Frame_12;
	qtarch_Frame_12 = new QFrame( this, "Frame_12" );
	qtarch_Frame_12->setGeometry( 210, 60, 140, 10 );
	qtarch_Frame_12->setMinimumSize( 0, 0 );
	qtarch_Frame_12->setMaximumSize( 32767, 32767 );
	qtarch_Frame_12->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_12->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_12->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_12->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_12->setFrameStyle( 36 );

	QFrame* qtarch_Frame_8;
	qtarch_Frame_8 = new QFrame( this, "Frame_8" );
	qtarch_Frame_8->setGeometry( 90, 60, 150, 10 );
	qtarch_Frame_8->setMinimumSize( 0, 0 );
	qtarch_Frame_8->setMaximumSize( 32767, 32767 );
	qtarch_Frame_8->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_8->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_8->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_8->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_8->setFrameStyle( 36 );

	QFrame* qtarch_Frame_9;
	qtarch_Frame_9 = new QFrame( this, "Frame_9" );
	qtarch_Frame_9->setGeometry( 330, 60, 10, 70 );
	qtarch_Frame_9->setMinimumSize( 0, 0 );
	qtarch_Frame_9->setMaximumSize( 32767, 32767 );
	qtarch_Frame_9->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_9->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_9->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_9->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_9->setFrameStyle( 37 );

	QFrame* qtarch_Frame_7;
	qtarch_Frame_7 = new QFrame( this, "Frame_7" );
	qtarch_Frame_7->setGeometry( 70, 0, 10, 180 );
	qtarch_Frame_7->setMinimumSize( 0, 0 );
	qtarch_Frame_7->setMaximumSize( 32767, 32767 );
	qtarch_Frame_7->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_7->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_7->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_7->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_7->setFrameStyle( 37 );

	QFrame* qtarch_Frame_2;
	qtarch_Frame_2 = new QFrame( this, "Frame_2" );
	qtarch_Frame_2->setGeometry( 10, 190, 380, 100 );
	qtarch_Frame_2->setMinimumSize( 0, 0 );
	qtarch_Frame_2->setMaximumSize( 32767, 32767 );
	qtarch_Frame_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_2->setFrameStyle( 33 );

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 70, 200, 310, 60 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( "Open DataBase Connectivity (ODBC) was developed to be an Open and portable standard for accessing data. unixODBC implements this standard for Linux/UNIX." );
	qtarch_Label_1->setAlignment( 1313 );
	qtarch_Label_1->setMargin( -1 );

	QLabel* qtarch_Label_2;
	qtarch_Label_2 = new QLabel( this, "Label_2" );
	qtarch_Label_2->setGeometry( 30, 210, 32, 32 );
	qtarch_Label_2->setMinimumSize( 32, 32 );
	qtarch_Label_2->setMaximumSize( 32, 32 );
	qtarch_Label_2->setBackgroundPixmap( QPixmap( info_xpm ) );
	qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_2->setText( "" );
	qtarch_Label_2->setAlignment( 289 );
	qtarch_Label_2->setMargin( -1 );

	QPushButton* qtarch_pbApplication;
	qtarch_pbApplication = new QPushButton( this, "pbApplication" );
	qtarch_pbApplication->setGeometry( 20, 0, 110, 30 );
	qtarch_pbApplication->setMinimumSize( 0, 0 );
	qtarch_pbApplication->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbApplication, SIGNAL(clicked()), SLOT(pbApplication_Clicked()) );
	qtarch_pbApplication->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbApplication->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbApplication->setFontPropagation( QWidget::NoChildren );
	qtarch_pbApplication->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbApplication->setText( "Application" );
	qtarch_pbApplication->setAutoRepeat( FALSE );
	qtarch_pbApplication->setAutoResize( FALSE );

	QPushButton* qtarch_pbDriverManager;
	qtarch_pbDriverManager = new QPushButton( this, "pbDriverManager" );
	qtarch_pbDriverManager->setGeometry( 20, 50, 110, 30 );
	qtarch_pbDriverManager->setMinimumSize( 0, 0 );
	qtarch_pbDriverManager->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbDriverManager, SIGNAL(clicked()), SLOT(pbDriverManager_Clicked()) );
	qtarch_pbDriverManager->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbDriverManager->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbDriverManager->setFontPropagation( QWidget::NoChildren );
	qtarch_pbDriverManager->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbDriverManager->setText( "Driver Manager" );
	qtarch_pbDriverManager->setAutoRepeat( FALSE );
	qtarch_pbDriverManager->setAutoResize( FALSE );

	QPushButton* qtarch_pbDriver;
	qtarch_pbDriver = new QPushButton( this, "pbDriver" );
	qtarch_pbDriver->setGeometry( 20, 100, 110, 30 );
	qtarch_pbDriver->setMinimumSize( 0, 0 );
	qtarch_pbDriver->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbDriver, SIGNAL(clicked()), SLOT(pbDriver_Clicked()) );
	qtarch_pbDriver->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbDriver->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbDriver->setFontPropagation( QWidget::NoChildren );
	qtarch_pbDriver->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbDriver->setText( "Driver" );
	qtarch_pbDriver->setAutoRepeat( FALSE );
	qtarch_pbDriver->setAutoResize( FALSE );

	QPushButton* qtarch_pbDatabase;
	qtarch_pbDatabase = new QPushButton( this, "pbDatabase" );
	qtarch_pbDatabase->setGeometry( 20, 150, 110, 30 );
	qtarch_pbDatabase->setMinimumSize( 0, 0 );
	qtarch_pbDatabase->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbDatabase, SIGNAL(clicked()), SLOT(pbDatabase_Clicked()) );
	qtarch_pbDatabase->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbDatabase->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbDatabase->setFontPropagation( QWidget::NoChildren );
	qtarch_pbDatabase->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbDatabase->setText( "Database System" );
	qtarch_pbDatabase->setAutoRepeat( FALSE );
	qtarch_pbDatabase->setAutoResize( FALSE );

	QPushButton* qtarch_pbODBC;
	qtarch_pbODBC = new QPushButton( this, "pbODBC" );
	qtarch_pbODBC->setGeometry( 160, 50, 100, 30 );
	qtarch_pbODBC->setMinimumSize( 0, 0 );
	qtarch_pbODBC->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbODBC, SIGNAL(clicked()), SLOT(pbODBC_Clicked()) );
	qtarch_pbODBC->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbODBC->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbODBC->setFontPropagation( QWidget::NoChildren );
	qtarch_pbODBC->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbODBC->setText( "odbc.ini" );
	qtarch_pbODBC->setAutoRepeat( FALSE );
	qtarch_pbODBC->setAutoResize( FALSE );

	QPushButton* qtarch_pbODBCDrivers;
	qtarch_pbODBCDrivers = new QPushButton( this, "pbODBCDrivers" );
	qtarch_pbODBCDrivers->setGeometry( 290, 100, 100, 30 );
	qtarch_pbODBCDrivers->setMinimumSize( 0, 0 );
	qtarch_pbODBCDrivers->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbODBCDrivers, SIGNAL(clicked()), SLOT(pbODBCDrivers_Clicked()) );
	qtarch_pbODBCDrivers->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbODBCDrivers->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbODBCDrivers->setFontPropagation( QWidget::NoChildren );
	qtarch_pbODBCDrivers->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbODBCDrivers->setText( "odbcinst.ini" );
	qtarch_pbODBCDrivers->setAutoRepeat( FALSE );
	qtarch_pbODBCDrivers->setAutoResize( FALSE );

	QPushButton* qtarch_pbCredits;
	qtarch_pbCredits = new QPushButton( this, "pbCredits" );
	qtarch_pbCredits->setGeometry( 280, 250, 100, 30 );
	qtarch_pbCredits->setMinimumSize( 0, 0 );
	qtarch_pbCredits->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbCredits, SIGNAL(clicked()), SLOT(pbCredits_Clicked()) );
	qtarch_pbCredits->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbCredits->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbCredits->setFontPropagation( QWidget::NoChildren );
	qtarch_pbCredits->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbCredits->setText( "&Credits" );
	qtarch_pbCredits->setAutoRepeat( FALSE );
	qtarch_pbCredits->setAutoResize( FALSE );

	QLabel* qtarch_Label_9;
	qtarch_Label_9 = new QLabel( this, "Label_9" );
	qtarch_Label_9->setGeometry( 70, 258, 210, 30 );
	qtarch_Label_9->setMinimumSize( 0, 0 );
	qtarch_Label_9->setMaximumSize( 32767, 32767 );
	qtarch_Label_9->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_9->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_9->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_9->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_9->setText( "http://www.unixodbc.org" );
	qtarch_Label_9->setAlignment( 289 );
	qtarch_Label_9->setMargin( -1 );

	QPushButton* qtarch_pbConfig;
	qtarch_pbConfig = new QPushButton( this, "pbConfig" );
	qtarch_pbConfig->setGeometry( 290, 50, 100, 30 );
	qtarch_pbConfig->setMinimumSize( 0, 0 );
	qtarch_pbConfig->setMaximumSize( 32767, 32767 );
	connect( qtarch_pbConfig, SIGNAL(clicked()), SLOT(pbODBCConfig_Clicked()) );
	qtarch_pbConfig->setFocusPolicy( QWidget::TabFocus );
	qtarch_pbConfig->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_pbConfig->setFontPropagation( QWidget::NoChildren );
	qtarch_pbConfig->setPalettePropagation( QWidget::NoChildren );
	qtarch_pbConfig->setText( "Config" );
	qtarch_pbConfig->setAutoRepeat( FALSE );
	qtarch_pbConfig->setAutoResize( FALSE );

	resize( 400,300 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
}


classAbout::~classAbout()
{
}

void classAbout::pbODBC_Clicked()
{
    QString qsMsg;

    qsMsg = "This is the main configuration file for ODBC.\n";
    qsMsg += "It contains Data Source configuration. \n";
    qsMsg += "It is used by the Driver Manager to determine, from a given Data\n";
    qsMsg += "Source Name, such things as the name of the Driver.\n";
    qsMsg += "It is a simple text file but is configured using the ODBCConfig tool.\n";
    qsMsg += "The User data sources are typically stored in ~/.odbc.ini while the\n";
    qsMsg += "System data sources are stored in /etc/odbc.ini\n";
    QMessageBox::information( this, "ODBC Config - odbc.ini", qsMsg );
}

void classAbout::pbODBCConfig_Clicked()
{
    QString qsMsg;

    qsMsg = "This is the program you are using now. This\n";
    qsMsg += "program allows the user to easily configure ODBC.\n";
    QMessageBox::information( this, "ODBC Config", qsMsg );
}

void classAbout::pbDatabase_Clicked()
{
    QString qsMsg;

    qsMsg = "Perhaps the most common type of Database System today is an SQL Server. ";
    qsMsg += "\n\nSQL Servers with Heavy Functionality;";
    qsMsg += "\n  ADABAS-D";
    qsMsg += "\n  Empress";
    qsMsg += "\n  Informix";
    qsMsg += "\n  Sybase - www.sybase.com";
    qsMsg += "\n  Oracle - www.oracle.com";
    qsMsg += "\n\nSQL Servers with Lite Functionality;";
    qsMsg += "\n  MiniSQL";
    qsMsg += "\n  MySQL";
    qsMsg += "\n  Solid";
    qsMsg += "\n\nThe Database System may be running on the local machine or on a ";
    qsMsg += "remote machine. It may also store its information in a variety of\n";
    qsMsg += "ways. This does not matter to an ODBC application because the Driver\n";
    qsMsg += "Manager and the Driver provides a consistent interface to the Database System.\n";
    QMessageBox::information( this, "ODBC Config - Database System", qsMsg );
}

void classAbout::pbDriverManager_Clicked()
{
    QString qsMsg;

    qsMsg = "The Driver Manager carries out a number of functions such as;\n";
    qsMsg += "1. resolves Data Source Names (via odbcinst lib)\n";
    qsMsg += "2. loads any required drivers\n";
    qsMsg += "3. calls the drivers exposed functions to communicate with the database\n";
    qsMsg += "Some functionality, such as listing all Data Sources, is only present\n";
    qsMsg += "in the Driver Manager (or via odbcinst lib).\n";
    qsMsg += "\n";
    qsMsg += "The DriverManager is the hub of ODBC activity... a traffic cop.\n";
    QMessageBox::information( this, "ODBC Config - Driver Manager", qsMsg );
}

void classAbout::pbDriver_Clicked()
{
    QString qsMsg;

    qsMsg = "The ODBC Drivers contain code specific to a Database\n";
    qsMsg += "System and provides a set of callable functions to the\n";
    qsMsg += "Driver Manager.\n";
    qsMsg += "Drivers may implement some database functionality when it\n";
    qsMsg += "is required by ODBC and is not present in the Database System.\n";
    qsMsg += "Drivers may also translate data types. Drivers, typically,\n";
    qsMsg += " do most of the ODBC work.\n";
    qsMsg += "\n";
    qsMsg += "ODBC Drivers can be obtained from the Internet or directly\n";
    qsMsg += "from the Database vendor.\n";
    qsMsg += "\n";
    qsMsg += "Check http://www.unixodbc.org for drivers\n";
    QMessageBox::information( this, "ODBC Config - Drivers", qsMsg );
}

void classAbout::pbODBCDrivers_Clicked()
{
    QString qsMsg;

    qsMsg = "odbcinst.ini contains a list of all installed ODBC\n";
    qsMsg += "Drivers. Each entry also contains some information\n";
    qsMsg += "about the driver such as the file name(s) of the driver.\n";
    qsMsg += "\n";
    qsMsg += "An entry should be made when an ODBC driver is installed\n";
    qsMsg += "and removed when the driver is uninstalled. This\n";
    qsMsg += "can be done using ODBCConfig or the odbcinst command tool.\n";
    QMessageBox::information( this, "ODBC Config - odbcinst.ini", qsMsg );
}

void classAbout::pbCredits_Clicked()
{
    QString qsMsg;

    qsMsg += "This program was derived from unixODBC. KDE also\n";
    qsMsg += "includes other parts of unixODBC such as the\n";
    qsMsg += "DriverManager and some supporting libs.\n";
    qsMsg += "\n";
    qsMsg += "Nick Gorham (nick@lurcher.org)\n";
    qsMsg += "- second project lead\n";
    qsMsg += "- complete rewrite of Driver Manager\n";
    qsMsg += "\n";
    qsMsg += "Peter Harvey (pharvey@codebydesign.com)\n";
    qsMsg += "- first project lead\n";
    qsMsg += "- original creator of most material\n";
    qsMsg += "\n";
    qsMsg += "Thanks to folks like Lars Doelle for helping out.\n";

    QMessageBox::information( this, "ODBC Config - Credits", qsMsg );
}

void classAbout::pbApplication_Clicked()
{
    QString qsMsg;

    qsMsg = "The Application communicates with the Driver Manager\n";
    qsMsg += "using the standard ODBC calls.\n";
    qsMsg += "The Application does not care; where the data is stored,\n";
    qsMsg += "how it is stored or even how the system is configured to\n";
    qsMsg += "access the data.\n";
    qsMsg += "The Application only needs to know the Data Source Name (DSN).\n";
    qsMsg += "\n";
    qsMsg += "The Application is NOT hard-wired to a particular Database\n";
    qsMsg += "System. This allows the user to choose a different Database\n";
    qsMsg += "System using the ODBCConfig tool.\n";

    QMessageBox::information( this, "ODBC Config - Application", qsMsg );
}


void classAbout::applySettings()
{
}

void classAbout::loadSettings()
{
}


