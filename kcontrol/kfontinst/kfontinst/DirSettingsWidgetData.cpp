#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'DirSettingsWidget.ui'
**
** Created: Fri Sep 7 00:45:22 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "DirSettingsWidgetData.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CDirSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CDirSettingsWidgetData::CDirSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CDirSettingsWidgetData" );
    resize( 482, 466 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, sizePolicy().hasHeightForWidth() ) );
    setCaption( i18n( "Form1" ) );
    CDirSettingsWidgetDataLayout = new QGridLayout( this ); 
    CDirSettingsWidgetDataLayout->setSpacing( 6 );
    CDirSettingsWidgetDataLayout->setMargin( 11 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CDirSettingsWidgetDataLayout->addItem( spacer, 3, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CDirSettingsWidgetDataLayout->addItem( spacer_2, 1, 1 );

    GroupBox4_3 = new QGroupBox( this, "GroupBox4_3" );
    GroupBox4_3->setTitle( i18n( "X:" ) );
    GroupBox4_3->setColumnLayout(0, Qt::Vertical );
    GroupBox4_3->layout()->setSpacing( 0 );
    GroupBox4_3->layout()->setMargin( 0 );
    GroupBox4_3Layout = new QGridLayout( GroupBox4_3->layout() );
    GroupBox4_3Layout->setAlignment( Qt::AlignTop );
    GroupBox4_3Layout->setSpacing( 6 );
    GroupBox4_3Layout->setMargin( 11 );

    GroupBox3 = new QGroupBox( GroupBox4_3, "GroupBox3" );
    GroupBox3->setTitle( i18n( "Basic Mode Sub-Folders:" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 0 );
    GroupBox3->layout()->setMargin( 0 );
    GroupBox3Layout = new QGridLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );
    GroupBox3Layout->setSpacing( 6 );
    GroupBox3Layout->setMargin( 11 );

    itsTTCombo = new QComboBox( FALSE, GroupBox3, "itsTTCombo" );
    itsTTCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, itsTTCombo->sizePolicy().hasHeightForWidth() ) );
    QToolTip::add(  itsTTCombo, i18n( "Where TrueType fonts are installed in \"basic\" mode." ) );

    GroupBox3Layout->addWidget( itsTTCombo, 0, 1 );

    itsT1Combo = new QComboBox( FALSE, GroupBox3, "itsT1Combo" );
    itsT1Combo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, itsT1Combo->sizePolicy().hasHeightForWidth() ) );
    QToolTip::add(  itsT1Combo, i18n( "Where Type1 fonts are installed in \"basic\" mode." ) );

    GroupBox3Layout->addWidget( itsT1Combo, 1, 1 );

    TextLabel1_2_2_2 = new QLabel( GroupBox3, "TextLabel1_2_2_2" );
    TextLabel1_2_2_2->setText( i18n( "Type1:" ) );

    GroupBox3Layout->addWidget( TextLabel1_2_2_2, 1, 0 );

    TextLabel1_2_3 = new QLabel( GroupBox3, "TextLabel1_2_3" );
    TextLabel1_2_3->setText( i18n( "TrueType :" ) );

    GroupBox3Layout->addWidget( TextLabel1_2_3, 0, 0 );

    GroupBox4_3Layout->addWidget( GroupBox3, 1, 1 );

    itsFontsDirText = new QLabel( GroupBox4_3, "itsFontsDirText" );
    itsFontsDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, itsFontsDirText->sizePolicy().hasHeightForWidth() ) );
    itsFontsDirText->setFrameShape( QLabel::Panel );
    itsFontsDirText->setFrameShadow( QLabel::Sunken );
    itsFontsDirText->setText( i18n( "TextLabel1" ) );
    itsFontsDirText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add(  itsFontsDirText, i18n( "This is the top level X11 fonts folder. Located\nwithin this will be the folders containing your\nfonts, and encodings files. Usually this will be\n(for Linux) either:\n\n    ~/.kde/share/config/fonts/    (Normal users)\n    /usr/X11R6/lib/X11/fonts/    (Root)" ) );

    GroupBox4_3Layout->addWidget( itsFontsDirText, 0, 1 );

    itsFontsDirButton = new QPushButton( GroupBox4_3, "itsFontsDirButton" );
    itsFontsDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, itsFontsDirButton->sizePolicy().hasHeightForWidth() ) );
    itsFontsDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsFontsDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsFontsDirButton->setText( i18n( "." ) );
    QToolTip::add(  itsFontsDirButton, i18n( "Change Folder." ) );

    GroupBox4_3Layout->addWidget( itsFontsDirButton, 0, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    GroupBox4_3Layout->addItem( spacer_3, 2, 1 );
    QSpacerItem* spacer_4 = new QSpacerItem( 36, 16, QSizePolicy::Fixed, QSizePolicy::Minimum );
    GroupBox4_3Layout->addItem( spacer_4, 1, 2 );

    TextLabel1_3 = new QLabel( GroupBox4_3, "TextLabel1_3" );
    TextLabel1_3->setText( i18n( "Fonts folder:" ) );

    GroupBox4_3Layout->addWidget( TextLabel1_3, 0, 0 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    GroupBox4_3Layout->addItem( spacer_5, 1, 0 );

    itsEncodingsDirButton = new QPushButton( GroupBox4_3, "itsEncodingsDirButton" );
    itsEncodingsDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, itsEncodingsDirButton->sizePolicy().hasHeightForWidth() ) );
    itsEncodingsDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsEncodingsDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsEncodingsDirButton->setText( i18n( "." ) );
    QToolTip::add(  itsEncodingsDirButton, i18n( "Change Folder." ) );

    GroupBox4_3Layout->addWidget( itsEncodingsDirButton, 7, 2 );

    itsEncodingsDirText = new QLabel( GroupBox4_3, "itsEncodingsDirText" );
    itsEncodingsDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, itsEncodingsDirText->sizePolicy().hasHeightForWidth() ) );
    itsEncodingsDirText->setFrameShape( QLabel::Panel );
    itsEncodingsDirText->setFrameShadow( QLabel::Sunken );
    itsEncodingsDirText->setText( i18n( "TextLabel1" ) );
    itsEncodingsDirText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add(  itsEncodingsDirText, i18n( "This is the folder containing XFree86 encoding\nfiles. These files (which usually have the suffix\n.enc or .ec.gz) are used to enable X to have\naccess to more encoding shemes (such as\ncp12525).\n\nThis folder is normaly a sub-folder of the system\nX11 fonts folder - and is usually named 'Encodings'\nor 'encodings'. As an example, on SuSE Linux\n7.1 - the folder is:\n\n/usr/X11R6/lib/X11/fonts/encodings/" ) );

    GroupBox4_3Layout->addWidget( itsEncodingsDirText, 7, 1 );

    TextLabel1_3_2 = new QLabel( GroupBox4_3, "TextLabel1_3_2" );
    TextLabel1_3_2->setText( i18n( "Encodings folder:" ) );

    GroupBox4_3Layout->addWidget( TextLabel1_3_2, 7, 0 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    GroupBox4_3Layout->addItem( spacer_6, 4, 1 );

    itsXConfigFileText = new QLabel( GroupBox4_3, "itsXConfigFileText" );
    itsXConfigFileText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, itsXConfigFileText->sizePolicy().hasHeightForWidth() ) );
    itsXConfigFileText->setFrameShape( QLabel::Panel );
    itsXConfigFileText->setFrameShadow( QLabel::Sunken );
    itsXConfigFileText->setText( i18n( "TextLabel2" ) );
    itsXConfigFileText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add(  itsXConfigFileText, i18n( "This is the configuration file used by X to\ndetermine where to look for fonts.\n\nFor normal (non-root) users, this file will\nusually be called:\n    ~/.kde/share/config/fonts/fontpaths\n\nFor root, on Linux with XFree86, this file will\nusually be called XF86Config.\n\nIf the file is not specified, is of an unrecognized\nformat, or the Font Installer cannot write to the\nspecified file, then you will not be able to add,\nremove, or disable folders in the Advanced\nmode." ) );

    GroupBox4_3Layout->addWidget( itsXConfigFileText, 3, 1 );

    TextLabel2_2_2 = new QLabel( GroupBox4_3, "TextLabel2_2_2" );
    TextLabel2_2_2->setText( i18n( "Configuration file:" ) );
    TextLabel2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );

    GroupBox4_3Layout->addWidget( TextLabel2_2_2, 3, 0 );

    itsXConfigFileButton = new QPushButton( GroupBox4_3, "itsXConfigFileButton" );
    itsXConfigFileButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, itsXConfigFileButton->sizePolicy().hasHeightForWidth() ) );
    itsXConfigFileButton->setMinimumSize( QSize( 22, 22 ) );
    itsXConfigFileButton->setMaximumSize( QSize( 22, 22 ) );
    itsXConfigFileButton->setText( i18n( "." ) );
    QToolTip::add(  itsXConfigFileButton, i18n( "Select File." ) );

    GroupBox4_3Layout->addWidget( itsXConfigFileButton, 3, 2 );

    CDirSettingsWidgetDataLayout->addMultiCellWidget( GroupBox4_3, 0, 0, 0, 2 );

    itsGhostscriptCheck = new QCheckBox( this, "itsGhostscriptCheck" );
    itsGhostscriptCheck->setText( i18n( "Ghostscript, Fontmap file:" ) );
    QWhatsThis::add(  itsGhostscriptCheck, i18n( "Check this box to enable configuration\nof Ghostscript." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsGhostscriptCheck, 2, 0 );

    itsGhostscriptFileText = new QLabel( this, "itsGhostscriptFileText" );
    itsGhostscriptFileText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, itsGhostscriptFileText->sizePolicy().hasHeightForWidth() ) );
    itsGhostscriptFileText->setFrameShape( QLabel::Panel );
    itsGhostscriptFileText->setFrameShadow( QLabel::Sunken );
    itsGhostscriptFileText->setText( i18n( "TextLabel2" ) );
    itsGhostscriptFileText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add(  itsGhostscriptFileText, i18n( "This file is used by Ghostscript to map the\nnames of fonts within Postscript files, to\nthe physical fonts located on disk.\n\n(Most UNIX/Linux programs produce\nPostscript files when printing - and these\nare passed on to Ghostscript, which does\nthe actual work.)\n\nFor normal (non-root) users, this file will\nusually be:\n    ~/.kde/share/config/fonts/Fontmap\n\n...and for root, usually the system-wide\nFontmap will be selected, and this would\nnormally be something like:\n    /usr/share/ghostscript/5.50/Fontmap" ) );

    CDirSettingsWidgetDataLayout->addWidget( itsGhostscriptFileText, 2, 1 );

    itsGhostscriptFileButton = new QPushButton( this, "itsGhostscriptFileButton" );
    itsGhostscriptFileButton->setEnabled( FALSE );
    itsGhostscriptFileButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, itsGhostscriptFileButton->sizePolicy().hasHeightForWidth() ) );
    itsGhostscriptFileButton->setMinimumSize( QSize( 22, 22 ) );
    itsGhostscriptFileButton->setMaximumSize( QSize( 22, 22 ) );
    itsGhostscriptFileButton->setText( i18n( "." ) );
    QToolTip::add(  itsGhostscriptFileButton, i18n( "Select File." ) );

    CDirSettingsWidgetDataLayout->addWidget( itsGhostscriptFileButton, 2, 2 );

    // signals and slots connections
    connect( itsFontsDirButton, SIGNAL( clicked() ), this, SLOT( xDirButtonPressed() ) );
    connect( itsEncodingsDirButton, SIGNAL( clicked() ), this, SLOT( encodingsDirButtonPressed() ) );
    connect( itsGhostscriptFileButton, SIGNAL( clicked() ), this, SLOT( gsFontmapButtonPressed() ) );
    connect( itsT1Combo, SIGNAL( activated(const QString&) ), this, SLOT( t1SubDir(const QString &) ) );
    connect( itsTTCombo, SIGNAL( activated(const QString&) ), this, SLOT( ttSubDir(const QString &) ) );
    connect( itsXConfigFileButton, SIGNAL( clicked() ), this, SLOT( xConfigButtonPressed() ) );
    connect( itsGhostscriptCheck, SIGNAL( toggled(bool) ), itsGhostscriptFileButton, SLOT( setEnabled(bool) ) );
    connect( itsGhostscriptCheck, SIGNAL( toggled(bool) ), this, SLOT( ghostscriptChecked(bool) ) );

    // tab order
    setTabOrder( itsFontsDirButton, itsTTCombo );
    setTabOrder( itsTTCombo, itsT1Combo );
    setTabOrder( itsT1Combo, itsXConfigFileButton );
    setTabOrder( itsXConfigFileButton, itsEncodingsDirButton );
    setTabOrder( itsEncodingsDirButton, itsGhostscriptFileButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CDirSettingsWidgetData::~CDirSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
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
