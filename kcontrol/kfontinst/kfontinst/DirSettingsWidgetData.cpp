#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'DirSettingsWidget.ui'
**
** Created: Tue Jan 8 22:18:26 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "DirSettingsWidgetData.h"

#include <qvariant.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CDirSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CDirSettingsWidgetData::CDirSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CDirSettingsWidgetData" );
    resize( 497, 403 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setCaption( tr2i18n( "Form1" ) );
    CDirSettingsWidgetDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CDirSettingsWidgetDataLayout"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CDirSettingsWidgetDataLayout->addItem( spacer, 4, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CDirSettingsWidgetDataLayout->addItem( spacer_2, 1, 1 );

    GroupBox4_3 = new QGroupBox( this, "GroupBox4_3" );
    GroupBox4_3->setTitle( tr2i18n( "X Settings" ) );
    GroupBox4_3->setColumnLayout(0, Qt::Vertical );
    GroupBox4_3->layout()->setSpacing( 6 );
    GroupBox4_3->layout()->setMargin( 11 );
    GroupBox4_3Layout = new QGridLayout( GroupBox4_3->layout() );
    GroupBox4_3Layout->setAlignment( Qt::AlignTop );

    GroupBox3 = new QGroupBox( GroupBox4_3, "GroupBox3" );
    GroupBox3->setTitle( tr2i18n( "Basic Mode Sub-Folders" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 6 );
    GroupBox3->layout()->setMargin( 11 );
    GroupBox3Layout = new QGridLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );

    itsTTCombo = new QComboBox( FALSE, GroupBox3, "itsTTCombo" );
    itsTTCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsTTCombo->sizePolicy().hasHeightForWidth() ) );
    QToolTip::add( itsTTCombo, tr2i18n( "Where TrueType fonts are installed in \"basic\" mode." ) );

    GroupBox3Layout->addWidget( itsTTCombo, 0, 1 );

    itsT1Combo = new QComboBox( FALSE, GroupBox3, "itsT1Combo" );
    itsT1Combo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsT1Combo->sizePolicy().hasHeightForWidth() ) );
    QToolTip::add( itsT1Combo, tr2i18n( "Where Type1 fonts are installed in \"basic\" mode." ) );

    GroupBox3Layout->addWidget( itsT1Combo, 1, 1 );

    TextLabel1_2_2_2 = new QLabel( GroupBox3, "TextLabel1_2_2_2" );
    TextLabel1_2_2_2->setText( tr2i18n( "Type1:" ) );

    GroupBox3Layout->addWidget( TextLabel1_2_2_2, 1, 0 );

    TextLabel1_2_3 = new QLabel( GroupBox3, "TextLabel1_2_3" );
    TextLabel1_2_3->setText( tr2i18n( "TrueType :" ) );

    GroupBox3Layout->addWidget( TextLabel1_2_3, 0, 0 );

    GroupBox4_3Layout->addWidget( GroupBox3, 1, 1 );

    itsFontsDirText = new QLabel( GroupBox4_3, "itsFontsDirText" );
    itsFontsDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsFontsDirText->sizePolicy().hasHeightForWidth() ) );
    itsFontsDirText->setFrameShape( QLabel::Panel );
    itsFontsDirText->setFrameShadow( QLabel::Sunken );
    itsFontsDirText->setText( tr2i18n( "TextLabel1" ) );
    itsFontsDirText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add( itsFontsDirText, tr2i18n( "This is the top level X11 fonts folder. Located within this will be the folders containing your fonts, and encodings files. Usually this will be (for Linux) either:\n"
"\n"
"    ~/.kde/share/fonts/    (Normal users)\n"
"    /usr/X11R6/lib/X11/fonts/    (Root)" ) );

    GroupBox4_3Layout->addWidget( itsFontsDirText, 0, 1 );

    itsFontsDirButton = new QPushButton( GroupBox4_3, "itsFontsDirButton" );
    itsFontsDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsFontsDirButton->sizePolicy().hasHeightForWidth() ) );
    itsFontsDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsFontsDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsFontsDirButton->setText( tr2i18n( "." ) );
    QToolTip::add( itsFontsDirButton, tr2i18n( "Change Folder." ) );

    GroupBox4_3Layout->addWidget( itsFontsDirButton, 0, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    GroupBox4_3Layout->addItem( spacer_3, 2, 1 );
    QSpacerItem* spacer_4 = new QSpacerItem( 36, 16, QSizePolicy::Fixed, QSizePolicy::Minimum );
    GroupBox4_3Layout->addItem( spacer_4, 1, 2 );

    TextLabel1_3 = new QLabel( GroupBox4_3, "TextLabel1_3" );
    TextLabel1_3->setText( tr2i18n( "Fonts folder:" ) );

    GroupBox4_3Layout->addWidget( TextLabel1_3, 0, 0 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    GroupBox4_3Layout->addItem( spacer_5, 1, 0 );

    itsEncodingsDirButton = new QPushButton( GroupBox4_3, "itsEncodingsDirButton" );
    itsEncodingsDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsEncodingsDirButton->sizePolicy().hasHeightForWidth() ) );
    itsEncodingsDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsEncodingsDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsEncodingsDirButton->setText( tr2i18n( "." ) );
    QToolTip::add( itsEncodingsDirButton, tr2i18n( "Change Folder." ) );

    GroupBox4_3Layout->addWidget( itsEncodingsDirButton, 7, 2 );

    itsEncodingsDirText = new QLabel( GroupBox4_3, "itsEncodingsDirText" );
    itsEncodingsDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsEncodingsDirText->sizePolicy().hasHeightForWidth() ) );
    itsEncodingsDirText->setFrameShape( QLabel::Panel );
    itsEncodingsDirText->setFrameShadow( QLabel::Sunken );
    itsEncodingsDirText->setText( tr2i18n( "TextLabel1" ) );
    itsEncodingsDirText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add( itsEncodingsDirText, tr2i18n( "This is the folder containing XFree86 encoding files. These files (which usually have the suffix .enc or .enc.gz) are used to enable X to have access to more encoding shemes (such as cp1252).\n"
"\n"
"This folder is normally a sub-folder of the system X11 fonts folder - and is usually named 'Encodings' or 'encodings'. As an example, on SuSE Linux 7.1 - the folder is:\n"
"\n"
"    /usr/X11R6/lib/X11/fonts/encodings/" ) );

    GroupBox4_3Layout->addWidget( itsEncodingsDirText, 7, 1 );

    TextLabel1_3_2 = new QLabel( GroupBox4_3, "TextLabel1_3_2" );
    TextLabel1_3_2->setText( tr2i18n( "Encodings folder:" ) );

    GroupBox4_3Layout->addWidget( TextLabel1_3_2, 7, 0 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    GroupBox4_3Layout->addItem( spacer_6, 4, 1 );

    itsXConfigFileText = new QLabel( GroupBox4_3, "itsXConfigFileText" );
    itsXConfigFileText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsXConfigFileText->sizePolicy().hasHeightForWidth() ) );
    itsXConfigFileText->setFrameShape( QLabel::Panel );
    itsXConfigFileText->setFrameShadow( QLabel::Sunken );
    itsXConfigFileText->setText( tr2i18n( "TextLabel2" ) );
    itsXConfigFileText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add( itsXConfigFileText, tr2i18n( "This is the configuration file used by X to determine where to look for fonts.\n"
"\n"
"For normal (non-root) users, this file will usually be called:\n"
"    ~/.kde/share/fonts/fontpaths\n"
"\n"
"For root, on Linux with XFree86, this file will usually be called XF86Config.\n"
"\n"
"If the file is not specified, is of an unrecognized format, or the Font Installer cannot write to the specified file, then you will not be able to add, remove, or disable folders in the Advanced mode." ) );

    GroupBox4_3Layout->addWidget( itsXConfigFileText, 3, 1 );

    TextLabel2_2_2 = new QLabel( GroupBox4_3, "TextLabel2_2_2" );
    TextLabel2_2_2->setText( tr2i18n( "Configuration file:" ) );
    TextLabel2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );

    GroupBox4_3Layout->addWidget( TextLabel2_2_2, 3, 0 );

    itsXConfigFileButton = new QPushButton( GroupBox4_3, "itsXConfigFileButton" );
    itsXConfigFileButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsXConfigFileButton->sizePolicy().hasHeightForWidth() ) );
    itsXConfigFileButton->setMinimumSize( QSize( 22, 22 ) );
    itsXConfigFileButton->setMaximumSize( QSize( 22, 22 ) );
    itsXConfigFileButton->setText( tr2i18n( "." ) );
    QToolTip::add( itsXConfigFileButton, tr2i18n( "Select File." ) );

    GroupBox4_3Layout->addWidget( itsXConfigFileButton, 3, 2 );

    CDirSettingsWidgetDataLayout->addMultiCellWidget( GroupBox4_3, 0, 0, 0, 2 );

    itsGhostscriptCheck = new QCheckBox( this, "itsGhostscriptCheck" );
    itsGhostscriptCheck->setText( tr2i18n( "Ghostscript, Fontmap file:" ) );
    QWhatsThis::add( itsGhostscriptCheck, tr2i18n( "Check this box to enable configuration of Ghostscript." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsGhostscriptCheck, 2, 0 );

    itsGhostscriptFileText = new QLabel( this, "itsGhostscriptFileText" );
    itsGhostscriptFileText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsGhostscriptFileText->sizePolicy().hasHeightForWidth() ) );
    itsGhostscriptFileText->setFrameShape( QLabel::Panel );
    itsGhostscriptFileText->setFrameShadow( QLabel::Sunken );
    itsGhostscriptFileText->setText( tr2i18n( "TextLabel2" ) );
    itsGhostscriptFileText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add( itsGhostscriptFileText, tr2i18n( "This file is used by Ghostscript to map the names of fonts within PostScript files to the physical fonts located on disk.\n"
"\n"
"(Most UNIX/Linux programs produce PostScript files when printing - and these are passed on to Ghostscript, which does the actual work.)\n"
"\n"
"For normal (non-root) users, this file will usually be:\n"
"\n"
"    ~/.kde/share/fonts/Fontmap\n"
"\n"
"...and for root, the system-wide Fontmap will usually be selected, which would normally be something like:\n"
"\n"
"    /usr/share/ghostscript/5.50/Fontmap" ) );

    CDirSettingsWidgetDataLayout->addWidget( itsGhostscriptFileText, 2, 1 );

    itsGhostscriptFileButton = new QPushButton( this, "itsGhostscriptFileButton" );
    itsGhostscriptFileButton->setEnabled( FALSE );
    itsGhostscriptFileButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsGhostscriptFileButton->sizePolicy().hasHeightForWidth() ) );
    itsGhostscriptFileButton->setMinimumSize( QSize( 22, 22 ) );
    itsGhostscriptFileButton->setMaximumSize( QSize( 22, 22 ) );
    itsGhostscriptFileButton->setText( tr2i18n( "." ) );
    QToolTip::add( itsGhostscriptFileButton, tr2i18n( "Select File." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsGhostscriptFileButton, 2, 2 );

    itsCupsDirText = new QLabel( this, "itsCupsDirText" );
    itsCupsDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsCupsDirText->sizePolicy().hasHeightForWidth() ) );
    itsCupsDirText->setFrameShape( QLabel::Panel );
    itsCupsDirText->setFrameShadow( QLabel::Sunken );
    itsCupsDirText->setText( tr2i18n( "TextLabel2" ) );
    itsCupsDirText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add( itsCupsDirText, tr2i18n( "This is the system CUPS installation folder. Generally this will be:\n"
"\n"
"    /usr/share/cups/\n"
"\n"
"This option is only applicable if you are using CUPS as your printing system.\n"
"\n"
"Only root may modify the CUPS settings, hence this option is only available if this module is run as root." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsCupsDirText, 3, 1 );

    itsCupsCheck = new QCheckBox( this, "itsCupsCheck" );
    itsCupsCheck->setText( tr2i18n( "CUPS folder:" ) );
    QWhatsThis::add( itsCupsCheck, tr2i18n( "Check this box to enable configuration of CUPS." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsCupsCheck, 3, 0 );

    itsCupsDirButton = new QPushButton( this, "itsCupsDirButton" );
    itsCupsDirButton->setEnabled( FALSE );
    itsCupsDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsCupsDirButton->sizePolicy().hasHeightForWidth() ) );
    itsCupsDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsCupsDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsCupsDirButton->setText( tr2i18n( "." ) );
    QToolTip::add( itsCupsDirButton, tr2i18n( "Select File." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsCupsDirButton, 3, 2 );

    // signals and slots connections
    connect( itsFontsDirButton, SIGNAL( clicked() ), this, SLOT( xDirButtonPressed() ) );
    connect( itsEncodingsDirButton, SIGNAL( clicked() ), this, SLOT( encodingsDirButtonPressed() ) );
    connect( itsGhostscriptFileButton, SIGNAL( clicked() ), this, SLOT( gsFontmapButtonPressed() ) );
    connect( itsT1Combo, SIGNAL( activated(const QString&) ), this, SLOT( t1SubDir(const QString&) ) );
    connect( itsTTCombo, SIGNAL( activated(const QString&) ), this, SLOT( ttSubDir(const QString&) ) );
    connect( itsXConfigFileButton, SIGNAL( clicked() ), this, SLOT( xConfigButtonPressed() ) );
    connect( itsGhostscriptCheck, SIGNAL( toggled(bool) ), itsGhostscriptFileButton, SLOT( setEnabled(bool) ) );
    connect( itsGhostscriptCheck, SIGNAL( toggled(bool) ), this, SLOT( ghostscriptChecked(bool) ) );
    connect( itsCupsDirButton, SIGNAL( clicked() ), this, SLOT( cupsButtonPressed() ) );
    connect( itsCupsCheck, SIGNAL( toggled(bool) ), this, SLOT( cupsChecked(bool) ) );
    connect( itsCupsCheck, SIGNAL( toggled(bool) ), itsCupsDirButton, SLOT( setEnabled(bool) ) );

    // tab order
    setTabOrder( itsFontsDirButton, itsTTCombo );
    setTabOrder( itsTTCombo, itsT1Combo );
    setTabOrder( itsT1Combo, itsXConfigFileButton );
    setTabOrder( itsXConfigFileButton, itsEncodingsDirButton );
    setTabOrder( itsEncodingsDirButton, itsGhostscriptCheck );
    setTabOrder( itsGhostscriptCheck, itsGhostscriptFileButton );
    setTabOrder( itsGhostscriptFileButton, itsCupsCheck );
    setTabOrder( itsCupsCheck, itsCupsDirButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CDirSettingsWidgetData::~CDirSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CDirSettingsWidgetData::cupsButtonPressed()
{
    qWarning( "CDirSettingsWidgetData::cupsButtonPressed(): Not implemented yet!" );
}

void CDirSettingsWidgetData::encodingsDirButtonPressed()
{
    qWarning( "CDirSettingsWidgetData::encodingsDirButtonPressed(): Not implemented yet!" );
}

void CDirSettingsWidgetData::ghostscriptChecked(bool)
{
    qWarning( "CDirSettingsWidgetData::ghostscriptChecked(bool): Not implemented yet!" );
}

void CDirSettingsWidgetData::gsFontmapButtonPressed()
{
    qWarning( "CDirSettingsWidgetData::gsFontmapButtonPressed(): Not implemented yet!" );
}

void CDirSettingsWidgetData::cupsChecked(bool)
{
    qWarning( "CDirSettingsWidgetData::cupsChecked(bool): Not implemented yet!" );
}

void CDirSettingsWidgetData::t1SubDir(const QString &)
{
    qWarning( "CDirSettingsWidgetData::t1SubDir(const QString &): Not implemented yet!" );
}

void CDirSettingsWidgetData::ttSubDir(const QString &)
{
    qWarning( "CDirSettingsWidgetData::ttSubDir(const QString &): Not implemented yet!" );
}

void CDirSettingsWidgetData::xConfigButtonPressed()
{
    qWarning( "CDirSettingsWidgetData::xConfigButtonPressed(): Not implemented yet!" );
}

void CDirSettingsWidgetData::xDirButtonPressed()
{
    qWarning( "CDirSettingsWidgetData::xDirButtonPressed(): Not implemented yet!" );
}

#include "DirSettingsWidgetData.moc"
