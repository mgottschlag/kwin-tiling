#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'StarOfficeSettingsWidget.ui'
**
** Created: Wed Nov 21 00:35:21 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "StarOfficeSettingsWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CStarOfficeSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CStarOfficeSettingsWidgetData::CStarOfficeSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CStarOfficeSettingsWidgetData" );
    resize( 438, 216 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setCaption( tr2i18n( "Form4" ) );
    CStarOfficeSettingsWidgetDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CStarOfficeSettingsWidgetDataLayout"); 

    itsCheck = new QCheckBox( this, "itsCheck" );
    itsCheck->setText( tr2i18n( "&Configure" ) );
    QWhatsThis::add( itsCheck, tr2i18n( "Select this box to enable StarOffice configuration." ) );

    CStarOfficeSettingsWidgetDataLayout->addWidget( itsCheck, 0, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CStarOfficeSettingsWidgetDataLayout->addItem( spacer, 4, 1 );

    itsDirText = new QLabel( this, "itsDirText" );
    itsDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsDirText->sizePolicy().hasHeightForWidth() ) );
    itsDirText->setMinimumSize( QSize( 0, 0 ) );
    itsDirText->setMaximumSize( QSize( 32767, 32767 ) );
    itsDirText->setFrameShape( QLabel::Panel );
    itsDirText->setFrameShadow( QLabel::Sunken );
    itsDirText->setMargin( 0 );
    itsDirText->setText( tr2i18n( "TextLabel3" ) );
    itsDirText->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );

    CStarOfficeSettingsWidgetDataLayout->addWidget( itsDirText, 2, 1 );

    TextLabel2_2_2 = new QLabel( this, "TextLabel2_2_2" );
    TextLabel2_2_2->setText( tr2i18n( "Folder:" ) );

    CStarOfficeSettingsWidgetDataLayout->addWidget( TextLabel2_2_2, 2, 0 );

    TextLabel1_3_3 = new QLabel( this, "TextLabel1_3_3" );
    TextLabel1_3_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, TextLabel1_3_3->sizePolicy().hasHeightForWidth() ) );
    TextLabel1_3_3->setFrameShape( QLabel::MShape );
    TextLabel1_3_3->setFrameShadow( QLabel::MShadow );
    TextLabel1_3_3->setText( tr2i18n( "Use printer file:" ) );
    TextLabel1_3_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    QWhatsThis::add( TextLabel1_3_3, tr2i18n( "This is the top level X11 fonts directory.\n"
"Located within this will be the directories\n"
"containing your fonts, and encodings\n"
"files. Usually this will be (for Linux):\n"
"\n"
"/usr/X11R6/lib/X11/fonts" ) );

    CStarOfficeSettingsWidgetDataLayout->addWidget( TextLabel1_3_3, 3, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CStarOfficeSettingsWidgetDataLayout->addItem( spacer_2, 6, 1 );

    itsPpdCombo = new QComboBox( FALSE, this, "itsPpdCombo" );
    itsPpdCombo->setEnabled( FALSE );
    itsPpdCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsPpdCombo->sizePolicy().hasHeightForWidth() ) );

    CStarOfficeSettingsWidgetDataLayout->addWidget( itsPpdCombo, 3, 1 );

    itsDirButton = new QPushButton( this, "itsDirButton" );
    itsDirButton->setEnabled( FALSE );
    itsDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsDirButton->sizePolicy().hasHeightForWidth() ) );
    itsDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsDirButton->setText( tr2i18n( "." ) );
    QToolTip::add( itsDirButton, tr2i18n( "Change Folder." ) );

    CStarOfficeSettingsWidgetDataLayout->addWidget( itsDirButton, 2, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 16, 21, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CStarOfficeSettingsWidgetDataLayout->addItem( spacer_3, 1, 0 );

    itsNote = new QLabel( this, "itsNote" );
    itsNote->setText( tr2i18n( "NOTE: This section is only relevant for StarOffice versions prior to v6. (It is also not needed for OpenOffice.)" ) );
    itsNote->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    CStarOfficeSettingsWidgetDataLayout->addMultiCellWidget( itsNote, 5, 5, 0, 2 );

    // signals and slots connections
    connect( itsCheck, SIGNAL( toggled(bool) ), itsDirButton, SLOT( setEnabled(bool) ) );
    connect( itsCheck, SIGNAL( toggled(bool) ), itsPpdCombo, SLOT( setEnabled(bool) ) );
    connect( itsCheck, SIGNAL( toggled(bool) ), this, SLOT( configureSelected(bool) ) );
    connect( itsDirButton, SIGNAL( clicked() ), this, SLOT( dirButtonPressed() ) );
    connect( itsPpdCombo, SIGNAL( activated(const QString&) ), this, SLOT( ppdSelected(const QString&) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CStarOfficeSettingsWidgetData::~CStarOfficeSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CStarOfficeSettingsWidgetData::configureSelected(bool)
{
    qWarning( "CStarOfficeSettingsWidgetData::configureSelected(bool): Not implemented yet!" );
}

void CStarOfficeSettingsWidgetData::dirButtonPressed()
{
    qWarning( "CStarOfficeSettingsWidgetData::dirButtonPressed(): Not implemented yet!" );
}

void CStarOfficeSettingsWidgetData::ppdSelected(const QString &)
{
    qWarning( "CStarOfficeSettingsWidgetData::ppdSelected(const QString &): Not implemented yet!" );
}

#include "StarOfficeSettingsWidgetData.moc"
