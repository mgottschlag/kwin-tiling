#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'XftConfigSettingsWidget.ui'
**
** Created: Fri Sep 7 00:51:09 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "XftConfigSettingsWidgetData.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CXftConfigSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CXftConfigSettingsWidgetData::CXftConfigSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CXftConfigSettingsWidgetData" );
    resize( 498, 361 ); 
    setCaption( i18n( "Form1" ) );
    CXftConfigSettingsWidgetDataLayout = new QGridLayout( this ); 
    CXftConfigSettingsWidgetDataLayout->setSpacing( 6 );
    CXftConfigSettingsWidgetDataLayout->setMargin( 11 );

    itsUseSubPixelHintingCheck = new QCheckBox( this, "itsUseSubPixelHintingCheck" );
    itsUseSubPixelHintingCheck->setEnabled( FALSE );
    itsUseSubPixelHintingCheck->setText( i18n( "&Use sub-pixel hinting" ) );
    QWhatsThis::add(  itsUseSubPixelHintingCheck, i18n( "Subpixel hinting uses colors instead of gray\npixels to do the anti-aliasing.\n\nThis may be of use if you have an LCD screen." ) );

    CXftConfigSettingsWidgetDataLayout->addMultiCellWidget( itsUseSubPixelHintingCheck, 4, 4, 0, 2 );

    TextLabel2_2_2 = new QLabel( this, "TextLabel2_2_2" );
    TextLabel2_2_2->setText( i18n( "Configuration file:" ) );
    TextLabel2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );

    CXftConfigSettingsWidgetDataLayout->addMultiCellWidget( TextLabel2_2_2, 2, 2, 0, 1 );

    itsFromText = new QLineEdit( this, "itsFromText" );
    itsFromText->setEnabled( FALSE );
    itsFromText->setText( i18n( "8.0" ) );
    itsFromText->setAlignment( int( QLineEdit::AlignRight ) );

    CXftConfigSettingsWidgetDataLayout->addWidget( itsFromText, 3, 2 );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setText( i18n( "pt, to" ) );

    CXftConfigSettingsWidgetDataLayout->addWidget( TextLabel2, 3, 3 );

    itsToText = new QLineEdit( this, "itsToText" );
    itsToText->setEnabled( FALSE );
    itsToText->setText( i18n( "15.0" ) );
    itsToText->setAlignment( int( QLineEdit::AlignRight ) );

    CXftConfigSettingsWidgetDataLayout->addWidget( itsToText, 3, 4 );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    TextLabel3->setText( i18n( "pt" ) );

    CXftConfigSettingsWidgetDataLayout->addWidget( TextLabel3, 3, 5 );

    itsConfigFileText = new QLabel( this, "itsConfigFileText" );
    itsConfigFileText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, itsConfigFileText->sizePolicy().hasHeightForWidth() ) );
    itsConfigFileText->setFrameShape( QLabel::Panel );
    itsConfigFileText->setFrameShadow( QLabel::Sunken );
    itsConfigFileText->setText( i18n( "TextLabel2" ) );
    itsConfigFileText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    QWhatsThis::add(  itsConfigFileText, i18n( "This is the configuration file used by the XRender\nextension. Under Linux/XFree86 this file will usually\nbe either; ~/.xftconfig - for personal settings - or /usr/X11R6/lib/X11/XftConfig - for system-wide\nsettings (usually only \"root\" can alter this file)." ) );

    CXftConfigSettingsWidgetDataLayout->addMultiCellWidget( itsConfigFileText, 2, 2, 2, 4 );

    itsConfigFileButton = new QPushButton( this, "itsConfigFileButton" );
    itsConfigFileButton->setEnabled( TRUE );
    itsConfigFileButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, itsConfigFileButton->sizePolicy().hasHeightForWidth() ) );
    itsConfigFileButton->setMinimumSize( QSize( 22, 22 ) );
    itsConfigFileButton->setMaximumSize( QSize( 22, 22 ) );
    itsConfigFileButton->setText( i18n( "." ) );
    QToolTip::add(  itsConfigFileButton, i18n( "Select File." ) );

    CXftConfigSettingsWidgetDataLayout->addWidget( itsConfigFileButton, 2, 5 );
    QSpacerItem* spacer = new QSpacerItem( 16, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CXftConfigSettingsWidgetDataLayout->addItem( spacer, 5, 1 );

    itsAdvancedButton = new QPushButton( this, "itsAdvancedButton" );
    itsAdvancedButton->setEnabled( TRUE );
    itsAdvancedButton->setText( i18n( "&Advanced..." ) );

    CXftConfigSettingsWidgetDataLayout->addMultiCellWidget( itsAdvancedButton, 6, 6, 3, 4 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    CXftConfigSettingsWidgetDataLayout->addMultiCell( spacer_2, 6, 6, 0, 2 );

    itsExcludeRangeCheck = new QCheckBox( this, "itsExcludeRangeCheck" );
    itsExcludeRangeCheck->setEnabled( FALSE );
    itsExcludeRangeCheck->setText( i18n( "E&xclude range:" ) );

    CXftConfigSettingsWidgetDataLayout->addMultiCellWidget( itsExcludeRangeCheck, 3, 3, 0, 1 );

    itsSaveButton = new QPushButton( this, "itsSaveButton" );
    itsSaveButton->setEnabled( FALSE );
    itsSaveButton->setText( i18n( "Save &Changes" ) );

    CXftConfigSettingsWidgetDataLayout->addMultiCellWidget( itsSaveButton, 7, 7, 3, 4 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CXftConfigSettingsWidgetDataLayout->addItem( spacer_3, 8, 4 );

    // signals and slots connections
    connect( itsConfigFileButton, SIGNAL( clicked() ), this, SLOT( fileButtonPressed() ) );
    connect( itsExcludeRangeCheck, SIGNAL( toggled(bool) ), this, SLOT( excludeRangeChecked(bool) ) );
    connect( itsFromText, SIGNAL( textChanged(const QString&) ), this, SLOT( fromChanged(const QString &) ) );
    connect( itsToText, SIGNAL( textChanged(const QString&) ), this, SLOT( toChanged(const QString &) ) );
    connect( itsUseSubPixelHintingCheck, SIGNAL( toggled(bool) ), this, SLOT( useSubPixelChecked(bool) ) );
    connect( itsAdvancedButton, SIGNAL( clicked() ), this, SLOT( advancedButtonPressed() ) );
    connect( itsSaveButton, SIGNAL( clicked() ), this, SLOT( saveButtonPressed() ) );

    // tab order
    setTabOrder( itsConfigFileButton, itsExcludeRangeCheck );
    setTabOrder( itsExcludeRangeCheck, itsFromText );
    setTabOrder( itsFromText, itsToText );
    setTabOrder( itsToText, itsUseSubPixelHintingCheck );
    setTabOrder( itsUseSubPixelHintingCheck, itsAdvancedButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CXftConfigSettingsWidgetData::~CXftConfigSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CXftConfigSettingsWidgetData::advancedButtonPressed()
{
    qWarning( "CXftConfigSettingsWidgetData::advancedButtonPressed(): Not implemented yet!" );
}

void CXftConfigSettingsWidgetData::excludeRangeChecked(bool)
{
    qWarning( "CXftConfigSettingsWidgetData::excludeRangeChecked(bool): Not implemented yet!" );
}

void CXftConfigSettingsWidgetData::fileButtonPressed()
{
    qWarning( "CXftConfigSettingsWidgetData::fileButtonPressed(): Not implemented yet!" );
}

void CXftConfigSettingsWidgetData::fromChanged(const QString &)
{
    qWarning( "CXftConfigSettingsWidgetData::fromChanged(const QString &): Not implemented yet!" );
}

void CXftConfigSettingsWidgetData::saveButtonPressed()
{
    qWarning( "CXftConfigSettingsWidgetData::saveButtonPressed(): Not implemented yet!" );
}

void CXftConfigSettingsWidgetData::toChanged(const QString &)
{
    qWarning( "CXftConfigSettingsWidgetData::toChanged(const QString &): Not implemented yet!" );
}

void CXftConfigSettingsWidgetData::useSubPixelChecked(bool)
{
    qWarning( "CXftConfigSettingsWidgetData::useSubPixelChecked(bool): Not implemented yet!" );
}

#include "XftConfigSettingsWidgetData.moc"
