#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'SettingsWizard.ui'
**
** Created: Wed Oct 24 23:34:06 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SettingsWizardData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include "DirSettingsWidget.h"
#include "StarOfficeSettingsWidget.h"
#include "XftConfigSettingsWidget.h"
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CSettingsWizardData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
CSettingsWizardData::CSettingsWizardData( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : KWizard( parent, name, modal, fl )
{
    if ( !name )
	setName( "CSettingsWizardData" );
    resize( 674, 522 ); 
    setCaption( tr2i18n( "Settings Wizard" ) );

    itsIntroPage = new QWidget( this, "itsIntroPage" );
    itsIntroPageLayout = new QGridLayout( itsIntroPage, 1, 1, 11, 6, "itsIntroPageLayout"); 

    itsMainText = new QLabel( itsIntroPage, "itsMainText" );
    itsMainText->setText( tr2i18n( "As this is the first time you have run KFontinst, there are some configuration settings that you will need to inform KFontinst of." ) );
    itsMainText->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsIntroPageLayout->addMultiCellWidget( itsMainText, 0, 0, 0, 2 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    itsIntroPageLayout->addItem( spacer, 5, 2 );

    TextLabel1 = new QLabel( itsIntroPage, "TextLabel1" );
    TextLabel1->setText( tr2i18n( "Please press \"Next>\" to proceed." ) );

    itsIntroPageLayout->addMultiCellWidget( TextLabel1, 4, 4, 0, 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    itsIntroPageLayout->addItem( spacer_2, 3, 0 );

    itsNonRootText = new QLabel( itsIntroPage, "itsNonRootText" );
    itsNonRootText->setText( tr2i18n( "<B>NOTE</B>\n"
"<p>As you are not logged in as \"root\":\n"
"<ol>\n"
"<li>The settings set here will refer to your own personal settings - any fonts installed by you will (normally) only be available to you. To install fonts system-wide, login as \"root\".</li>\n"
"<li>Any fonts installed will only be printable by applications that embed the fonts within their output. Basically, for applications such as StarOffice, please login as \"root\" to install your fonts.</li>\n"
"<li>If you are running a font server (xfs), then any fonts installed may not actually be usable - as normally only \"root\" may install fonts in this case.</li>\n"
"</ol>\n"
"To sumarise: If you have problems using, or printing, the fonts you have installed - then you should try re-installing them as \"root\" (This can be accomplished via the \"Administrator Mode\" button.)\n"
"</p>" ) );
    itsNonRootText->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    itsIntroPageLayout->addMultiCellWidget( itsNonRootText, 2, 2, 0, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    itsIntroPageLayout->addItem( spacer_3, 1, 1 );
    addPage( itsIntroPage, tr2i18n( "Welcome To KFontinst" ) );

    itsDirsAndFilesPage = new QWidget( this, "itsDirsAndFilesPage" );
    itsDirsAndFilesPageLayout = new QGridLayout( itsDirsAndFilesPage, 1, 1, 11, 6, "itsDirsAndFilesPageLayout"); 

    itsDirsAndFilesWidget = new CDirSettingsWidget( itsDirsAndFilesPage, "itsDirsAndFilesWidget" );
    itsDirsAndFilesWidget->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, itsDirsAndFilesWidget->sizePolicy().hasHeightForWidth() ) );

    itsDirsAndFilesPageLayout->addWidget( itsDirsAndFilesWidget, 0, 1 );

    itsFnFText = new QLabel( itsDirsAndFilesPage, "itsFnFText" );
    itsFnFText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)3, 0, 0, itsFnFText->sizePolicy().hasHeightForWidth() ) );
    itsFnFText->setMinimumSize( QSize( 160, 200 ) );
    itsFnFText->setMaximumSize( QSize( 160, 32767 ) );
    itsFnFText->setFrameShape( QLabel::Panel );
    itsFnFText->setFrameShadow( QLabel::Sunken );
    itsFnFText->setMargin( 10 );
    itsFnFText->setText( tr2i18n( "KFontinst has determined the following values for your X11 and Ghostscript Fontmap folders and files. Please enter the correct location for any entries containing \"<Not Found>\"" ) );
    itsFnFText->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsDirsAndFilesPageLayout->addWidget( itsFnFText, 0, 0 );
    addPage( itsDirsAndFilesPage, tr2i18n( "Folder/File Locations" ) );

    itsAAPage = new QWidget( this, "itsAAPage" );
    itsAAPageLayout = new QGridLayout( itsAAPage, 1, 1, 11, 6, "itsAAPageLayout"); 

    TextLabel1_2 = new QLabel( itsAAPage, "TextLabel1_2" );
    TextLabel1_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)3, 0, 0, TextLabel1_2->sizePolicy().hasHeightForWidth() ) );
    TextLabel1_2->setMinimumSize( QSize( 160, 200 ) );
    TextLabel1_2->setMaximumSize( QSize( 160, 32767 ) );
    TextLabel1_2->setFrameShape( QLabel::Panel );
    TextLabel1_2->setFrameShadow( QLabel::Sunken );
    TextLabel1_2->setMargin( 10 );
    TextLabel1_2->setText( tr2i18n( "If your system is capable of using the XRender extension for anti-aliasing fonts, then please enter the relevant data." ) );
    TextLabel1_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsAAPageLayout->addWidget( TextLabel1_2, 0, 0 );

    XftWizard = new CXftConfigSettingsWidget( itsAAPage, "XftWizard" );

    itsAAPageLayout->addWidget( XftWizard, 0, 1 );
    addPage( itsAAPage, tr2i18n( "Anti-Aliasing" ) );

    itsStarOfficePage = new QWidget( this, "itsStarOfficePage" );
    itsStarOfficePageLayout = new QGridLayout( itsStarOfficePage, 1, 1, 11, 6, "itsStarOfficePageLayout"); 

    itsSOWidget = new CStarOfficeSettingsWidget( itsStarOfficePage, "itsSOWidget" );
    itsSOWidget->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, itsSOWidget->sizePolicy().hasHeightForWidth() ) );

    itsStarOfficePageLayout->addWidget( itsSOWidget, 0, 1 );

    TextLabel1_2_2 = new QLabel( itsStarOfficePage, "TextLabel1_2_2" );
    TextLabel1_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)3, 0, 0, TextLabel1_2_2->sizePolicy().hasHeightForWidth() ) );
    TextLabel1_2_2->setMinimumSize( QSize( 160, 200 ) );
    TextLabel1_2_2->setMaximumSize( QSize( 160, 32767 ) );
    TextLabel1_2_2->setFrameShape( QLabel::Panel );
    TextLabel1_2_2->setFrameShadow( QLabel::Sunken );
    TextLabel1_2_2->setMargin( 10 );
    TextLabel1_2_2->setText( tr2i18n( "If you wish to enable configuration of StarOffice from KFontinst, then select the \"Configure\" option, and enter the location of the main StarOffice folder.\n"
"\n"
"NOTE: This is not needed for OpenOffice, and StarOffice version 6 onwards. The only thing that these two require is the existence of AFM files for Type1 fonts. This can be set using the \"System\" part of the \"Settings\" tab when the module has loaded." ) );
    TextLabel1_2_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignLeft ) );

    itsStarOfficePageLayout->addWidget( TextLabel1_2_2, 0, 0 );
    addPage( itsStarOfficePage, tr2i18n( "StarOffice" ) );

    itsCompletePage = new QWidget( this, "itsCompletePage" );
    itsCompletePageLayout = new QGridLayout( itsCompletePage, 1, 1, 11, 6, "itsCompletePageLayout"); 

    TextLabel1_3 = new QLabel( itsCompletePage, "TextLabel1_3" );
    TextLabel1_3->setText( tr2i18n( "<p>KFontinst has now determined all the setup information that it needs.</p>\n"
"\n"
"<p>This KControl module has two main modes of operation:\n"
"<ol>\n"
"<li><i>Basic:</i> The underlying folder structure is hidden - and you will only be able to install/uninstall TrueType and Type1 fonts.</li>\n"
"<li><i>Advanced:</i> This is for more experienced users, and displays the X fonts folder structure - allowing you to add/delete whole folders to/from the X font path. Using this mode you can also install/uninstall Speedo and Bitmap (pcf, bdf, and snf) fonts.</li>\n"
"</ol>\n"
"To switch between these modes, select the appropriate option from within the \"Appearance\" section of the \"Settings\" tab.</p>\n"
"<p><b>NOTE</b> As this is the first time KFontinst has been run by you, it has automatically marked the basic folders containing your TrueType and Type1 fonts as needing to be 'configured' - therefore, before closing this KControl module you will be prompted to \"Apply\" these changes.</p>\n"
"<p>For further detailed help on how to use this module, please refer to the on-line help documentation.</p>" ) );
    TextLabel1_3->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    itsCompletePageLayout->addWidget( TextLabel1_3, 0, 0 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    itsCompletePageLayout->addItem( spacer_4, 1, 0 );
    addPage( itsCompletePage, tr2i18n( "Setup Complete" ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CSettingsWizardData::~CSettingsWizardData()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "SettingsWizardData.moc"
