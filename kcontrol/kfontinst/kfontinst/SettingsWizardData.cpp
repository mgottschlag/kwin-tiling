#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'SettingsWizard.ui'
**
** Created: Wed Jul 18 23:07:27 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SettingsWizardData.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include "DirSettingsWidget.h"
#include "StarOfficeSettingsWidget.h"
#include "XftConfigSettingsWidget.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CSettingsWizardData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
CSettingsWizardData::CSettingsWizardData( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : KWizard( parent, name, modal, fl )
{
    if ( !name )
	setName( "CSettingsWizardData" );
    resize( 641, 436 ); 
    setCaption( i18n( "Settings Wizard" ) );

    itsIntroPage = new QWidget( this, "itsIntroPage" );
    itsIntroPageLayout = new QGridLayout( itsIntroPage ); 
    itsIntroPageLayout->setSpacing( 6 );
    itsIntroPageLayout->setMargin( 11 );

    itsMainText = new QLabel( itsIntroPage, "itsMainText" );
    itsMainText->setText( i18n( "As this is the first time you have run KFontinst, there are some configuration settings that you will need to inform KFontinst of." ) );
    itsMainText->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsIntroPageLayout->addMultiCellWidget( itsMainText, 0, 0, 0, 2 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    itsIntroPageLayout->addItem( spacer, 5, 2 );

    TextLabel1 = new QLabel( itsIntroPage, "TextLabel1" );
    TextLabel1->setText( i18n( "Please press \"Next>\" to proceed." ) );

    itsIntroPageLayout->addMultiCellWidget( TextLabel1, 4, 4, 0, 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    itsIntroPageLayout->addItem( spacer_2, 3, 0 );

    itsNonRootText = new QLabel( itsIntroPage, "itsNonRootText" );
    itsNonRootText->setText( i18n( "NOTE: As you are not logged in as \"root\", then the settings will refer to your own personal settings - any fonts installed by you will (normally) only be available to you. To install fonts system-wide, login as root." ) );
    itsNonRootText->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    itsIntroPageLayout->addMultiCellWidget( itsNonRootText, 2, 2, 0, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    itsIntroPageLayout->addItem( spacer_3, 1, 1 );
    addPage( itsIntroPage, i18n( "Welcome To KFontinst" ) );

    itsDirsAndFilesPage = new QWidget( this, "itsDirsAndFilesPage" );
    itsDirsAndFilesPageLayout = new QGridLayout( itsDirsAndFilesPage ); 
    itsDirsAndFilesPageLayout->setSpacing( 6 );
    itsDirsAndFilesPageLayout->setMargin( 11 );

    MyCustomWidget1 = new CDirSettingsWidget( itsDirsAndFilesPage, "MyCustomWidget1" );
    MyCustomWidget1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, MyCustomWidget1->sizePolicy().hasHeightForWidth() ) );

    itsDirsAndFilesPageLayout->addWidget( MyCustomWidget1, 0, 1 );

    TextLabel2 = new QLabel( itsDirsAndFilesPage, "TextLabel2" );
    TextLabel2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)3, TextLabel2->sizePolicy().hasHeightForWidth() ) );
    TextLabel2->setMinimumSize( QSize( 160, 200 ) );
    TextLabel2->setMaximumSize( QSize( 160, 32767 ) );
    TextLabel2->setFrameShape( QLabel::Panel );
    TextLabel2->setFrameShadow( QLabel::Sunken );
    TextLabel2->setMargin( 10 );
    TextLabel2->setText( i18n( "KFontinst has determined the following values for your X11 and Ghostscript Fontmap folders and files. Please enter the correct location for any entries containing \"<Not Found>\"" ) );
    TextLabel2->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsDirsAndFilesPageLayout->addWidget( TextLabel2, 0, 0 );
    addPage( itsDirsAndFilesPage, i18n( "Folder/File Locations" ) );

    itsAAPage = new QWidget( this, "itsAAPage" );
    itsAAPageLayout = new QGridLayout( itsAAPage ); 
    itsAAPageLayout->setSpacing( 6 );
    itsAAPageLayout->setMargin( 11 );

    TextLabel1_2 = new QLabel( itsAAPage, "TextLabel1_2" );
    TextLabel1_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)3, TextLabel1_2->sizePolicy().hasHeightForWidth() ) );
    TextLabel1_2->setMinimumSize( QSize( 160, 200 ) );
    TextLabel1_2->setMaximumSize( QSize( 160, 32767 ) );
    TextLabel1_2->setFrameShape( QLabel::Panel );
    TextLabel1_2->setFrameShadow( QLabel::Sunken );
    TextLabel1_2->setMargin( 10 );
    TextLabel1_2->setText( i18n( "If your system is capable of using the XRender extension for anti-aliasing fonts, then please enter the relevant data." ) );
    TextLabel1_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsAAPageLayout->addWidget( TextLabel1_2, 0, 0 );

    XftWizard = new CXftConfigSettingsWidget( itsAAPage, "XftWizard" );

    itsAAPageLayout->addWidget( XftWizard, 0, 1 );
    addPage( itsAAPage, i18n( "Anti-Aliasing" ) );

    itsStarOfficePage = new QWidget( this, "itsStarOfficePage" );
    itsStarOfficePageLayout = new QGridLayout( itsStarOfficePage ); 
    itsStarOfficePageLayout->setSpacing( 6 );
    itsStarOfficePageLayout->setMargin( 11 );

    itsSOWidget = new CStarOfficeSettingsWidget( itsStarOfficePage, "itsSOWidget" );
    itsSOWidget->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, itsSOWidget->sizePolicy().hasHeightForWidth() ) );

    itsStarOfficePageLayout->addWidget( itsSOWidget, 0, 1 );

    TextLabel1_2_2 = new QLabel( itsStarOfficePage, "TextLabel1_2_2" );
    TextLabel1_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)3, TextLabel1_2_2->sizePolicy().hasHeightForWidth() ) );
    TextLabel1_2_2->setMinimumSize( QSize( 160, 200 ) );
    TextLabel1_2_2->setMaximumSize( QSize( 160, 32767 ) );
    TextLabel1_2_2->setFrameShape( QLabel::Panel );
    TextLabel1_2_2->setFrameShadow( QLabel::Sunken );
    TextLabel1_2_2->setMargin( 10 );
    TextLabel1_2_2->setText( i18n( "If you wish to enable configuration of StarOffice from KFontinst, then select the \"Configure\" option, and enter the location of the main StarOffice folder.\n\nNOTE: This is not needed for OpenOffice, and StarOffice version 6 onwards. The only thing that these two require is the exsistence of AFM files for Type1 fonts. This can be set using the \"System\" part of the \"Settings\" tab when the module has loaded." ) );
    TextLabel1_2_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsStarOfficePageLayout->addWidget( TextLabel1_2_2, 0, 0 );
    addPage( itsStarOfficePage, i18n( "StarOffice" ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CSettingsWizardData::~CSettingsWizardData()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "SettingsWizardData.moc"
