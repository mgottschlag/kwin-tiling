#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'SysCfgSettingsWidget.ui'
**
** Created: Wed Oct 24 21:21:40 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SysCfgSettingsWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CSysCfgSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CSysCfgSettingsWidgetData::CSysCfgSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CSysCfgSettingsWidgetData" );
    resize( 579, 280 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setCaption( tr2i18n( "Form3" ) );
    CSysCfgSettingsWidgetDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CSysCfgSettingsWidgetDataLayout"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CSysCfgSettingsWidgetDataLayout->addItem( spacer, 2, 0 );

    GroupBox4_2 = new QGroupBox( this, "GroupBox4_2" );
    GroupBox4_2->setTitle( tr2i18n( "General:" ) );
    GroupBox4_2->setColumnLayout(0, Qt::Vertical );
    GroupBox4_2->layout()->setSpacing( 6 );
    GroupBox4_2->layout()->setMargin( 11 );
    GroupBox4_2Layout = new QGridLayout( GroupBox4_2->layout() );
    GroupBox4_2Layout->setAlignment( Qt::AlignTop );

    itsX11EncodingCheck = new QCheckBox( GroupBox4_2, "itsX11EncodingCheck" );
    itsX11EncodingCheck->setText( tr2i18n( "&Configure X to only use" ) );

    GroupBox4_2Layout->addWidget( itsX11EncodingCheck, 0, 0 );

    itsX11EncodingCombo = new QComboBox( FALSE, GroupBox4_2, "itsX11EncodingCombo" );
    itsX11EncodingCombo->setEnabled( FALSE );
    itsX11EncodingCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsX11EncodingCombo->sizePolicy().hasHeightForWidth() ) );

    GroupBox4_2Layout->addMultiCellWidget( itsX11EncodingCombo, 0, 0, 1, 2 );

    TextLabel1 = new QLabel( GroupBox4_2, "TextLabel1" );
    TextLabel1->setText( tr2i18n( "encoding" ) );

    GroupBox4_2Layout->addWidget( TextLabel1, 0, 3 );

    itsAfmEncodingCombo = new QComboBox( FALSE, GroupBox4_2, "itsAfmEncodingCombo" );
    itsAfmEncodingCombo->setEnabled( FALSE );
    itsAfmEncodingCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsAfmEncodingCombo->sizePolicy().hasHeightForWidth() ) );
    QWhatsThis::add( itsAfmEncodingCombo, tr2i18n( "AFMs (Adobe Font Metrics) are files that may be used by programs (such as StarOffice) to obtain detailed \"metrics\" (e.g. sizes) of each character in a font - so that it may be displayed/printed correctly. \n"
"\n"
"AFMs may only contain one encoding, so here you can specify which one should be used." ) );

    GroupBox4_2Layout->addMultiCellWidget( itsAfmEncodingCombo, 2, 2, 1, 2 );

    itsT1AfmCheck = new QCheckBox( GroupBox4_2, "itsT1AfmCheck" );
    itsT1AfmCheck->setEnabled( FALSE );
    itsT1AfmCheck->setText( tr2i18n( "Type&1" ) );
    QWhatsThis::add( itsT1AfmCheck, tr2i18n( "Check this item to enable creation of AFMs for Type1 fonts.\n"
"\n"
"These are required by StarOffice, and some other apps (such as AbiWord)." ) );

    GroupBox4_2Layout->addWidget( itsT1AfmCheck, 3, 2 );

    itsGenAfmsCheck = new QCheckBox( GroupBox4_2, "itsGenAfmsCheck" );
    itsGenAfmsCheck->setText( tr2i18n( "&Generate AFMs, with" ) );
    QWhatsThis::add( itsGenAfmsCheck, tr2i18n( "AFMs (Adobe Font Metrics) are files that\n"
"may be used by programs (such as\n"
"StarOffice) to obtain detailed \"metrics\"\n"
"(e.g. sizes) of each character in a font -\n"
"so that it may be displayed/printed\n"
"correctly.\n"
"\n"
"AFMs may only contain one encoding,\n"
"so here you can specify which one\n"
"should be used." ) );

    GroupBox4_2Layout->addWidget( itsGenAfmsCheck, 2, 0 );

    itsTtAfmCheck = new QCheckBox( GroupBox4_2, "itsTtAfmCheck" );
    itsTtAfmCheck->setEnabled( FALSE );
    itsTtAfmCheck->setText( tr2i18n( "T&rueType" ) );
    QWhatsThis::add( itsTtAfmCheck, tr2i18n( "Check this item to enable creation of AFM files for TrueType fonts.\n"
"\n"
"NOTE: StarOffice v6 (onwards), and OpenOffice, no longer require AFMs for TrueType fonts." ) );

    GroupBox4_2Layout->addWidget( itsTtAfmCheck, 3, 1 );

    TextLabel1_2 = new QLabel( GroupBox4_2, "TextLabel1_2" );
    TextLabel1_2->setText( tr2i18n( "encoding" ) );

    GroupBox4_2Layout->addWidget( TextLabel1_2, 2, 3 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    GroupBox4_2Layout->addItem( spacer_2, 1, 1 );

    CSysCfgSettingsWidgetDataLayout->addWidget( GroupBox4_2, 0, 0 );

    ButtonGroup5_2_2 = new QButtonGroup( this, "ButtonGroup5_2_2" );
    ButtonGroup5_2_2->setTitle( tr2i18n( "Command To Refresh X's Font List" ) );
    ButtonGroup5_2_2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup5_2_2->layout()->setSpacing( 6 );
    ButtonGroup5_2_2->layout()->setMargin( 11 );
    ButtonGroup5_2_2Layout = new QGridLayout( ButtonGroup5_2_2->layout() );
    ButtonGroup5_2_2Layout->setAlignment( Qt::AlignTop );

    itsXsetRadio = new QRadioButton( ButtonGroup5_2_2, "itsXsetRadio" );
    itsXsetRadio->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsXsetRadio->sizePolicy().hasHeightForWidth() ) );
    itsXsetRadio->setText( tr2i18n( "&xset fp rehash" ) );
    itsXsetRadio->setChecked( TRUE );

    ButtonGroup5_2_2Layout->addMultiCellWidget( itsXsetRadio, 0, 0, 0, 1 );

    itsXfsRadio = new QRadioButton( ButtonGroup5_2_2, "itsXfsRadio" );
    itsXfsRadio->setText( tr2i18n( "/e&tc/rc.d/init.d/xfs restart" ) );

    ButtonGroup5_2_2Layout->addMultiCellWidget( itsXfsRadio, 1, 1, 0, 1 );

    itsCustomRadio = new QRadioButton( ButtonGroup5_2_2, "itsCustomRadio" );
    itsCustomRadio->setText( tr2i18n( "C&ustom:" ) );

    ButtonGroup5_2_2Layout->addWidget( itsCustomRadio, 2, 0 );

    itsRestartXfsCommand = new QLineEdit( ButtonGroup5_2_2, "itsRestartXfsCommand" );
    itsRestartXfsCommand->setEnabled( FALSE );
    QWhatsThis::add( itsRestartXfsCommand, tr2i18n( "This is the command that can be issued upon successful system configuration to restart the X font server." ) );

    ButtonGroup5_2_2Layout->addWidget( itsRestartXfsCommand, 2, 1 );

    CSysCfgSettingsWidgetDataLayout->addWidget( ButtonGroup5_2_2, 1, 0 );

    // signals and slots connections
    connect( itsCustomRadio, SIGNAL( toggled(bool) ), itsRestartXfsCommand, SLOT( setEnabled(bool) ) );
    connect( itsX11EncodingCheck, SIGNAL( toggled(bool) ), this, SLOT( encodingSelected(bool) ) );
    connect( itsXsetRadio, SIGNAL( toggled(bool) ), this, SLOT( xsetFpRehashSelected(bool) ) );
    connect( itsXfsRadio, SIGNAL( toggled(bool) ), this, SLOT( xfsRestartSelected(bool) ) );
    connect( itsCustomRadio, SIGNAL( toggled(bool) ), this, SLOT( customXRefreshSelected(bool) ) );
    connect( itsRestartXfsCommand, SIGNAL( textChanged(const QString&) ), this, SLOT( customXStrChanged(const QString&) ) );
    connect( itsX11EncodingCombo, SIGNAL( activated(const QString&) ), this, SLOT( encodingSelected(const QString&) ) );
    connect( itsX11EncodingCheck, SIGNAL( toggled(bool) ), itsX11EncodingCombo, SLOT( setEnabled(bool) ) );
    connect( itsGenAfmsCheck, SIGNAL( toggled(bool) ), itsAfmEncodingCombo, SLOT( setEnabled(bool) ) );
    connect( itsGenAfmsCheck, SIGNAL( toggled(bool) ), this, SLOT( generateAfmsSelected(bool) ) );
    connect( itsAfmEncodingCombo, SIGNAL( activated(const QString&) ), this, SLOT( afmEncodingSelected(const QString&) ) );
    connect( itsTtAfmCheck, SIGNAL( toggled(bool) ), this, SLOT( ttAfmSelected(bool) ) );
    connect( itsT1AfmCheck, SIGNAL( toggled(bool) ), this, SLOT( t1AfmSelected(bool) ) );
    connect( itsGenAfmsCheck, SIGNAL( toggled(bool) ), itsTtAfmCheck, SLOT( setEnabled(bool) ) );
    connect( itsGenAfmsCheck, SIGNAL( toggled(bool) ), itsT1AfmCheck, SLOT( setEnabled(bool) ) );

    // tab order
    setTabOrder( itsX11EncodingCheck, itsX11EncodingCombo );
    setTabOrder( itsX11EncodingCombo, itsGenAfmsCheck );
    setTabOrder( itsGenAfmsCheck, itsAfmEncodingCombo );
    setTabOrder( itsAfmEncodingCombo, itsTtAfmCheck );
    setTabOrder( itsTtAfmCheck, itsT1AfmCheck );
    setTabOrder( itsT1AfmCheck, itsXsetRadio );
    setTabOrder( itsXsetRadio, itsXfsRadio );
    setTabOrder( itsXfsRadio, itsCustomRadio );
    setTabOrder( itsCustomRadio, itsRestartXfsCommand );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CSysCfgSettingsWidgetData::~CSysCfgSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CSysCfgSettingsWidgetData::afmEncodingSelected(const QString &)
{
    qWarning( "CSysCfgSettingsWidgetData::afmEncodingSelected(const QString &): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::customXRefreshSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::customXRefreshSelected(bool): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::customXStrChanged(const QString &)
{
    qWarning( "CSysCfgSettingsWidgetData::customXStrChanged(const QString &): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::encodingSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::encodingSelected(bool): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::encodingSelected(const QString &)
{
    qWarning( "CSysCfgSettingsWidgetData::encodingSelected(const QString &): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::generateAfmsSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::generateAfmsSelected(bool): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::t1AfmSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::t1AfmSelected(bool): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::ttAfmSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::ttAfmSelected(bool): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::xfsRestartSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::xfsRestartSelected(bool): Not implemented yet!" );
}

void CSysCfgSettingsWidgetData::xsetFpRehashSelected(bool)
{
    qWarning( "CSysCfgSettingsWidgetData::xsetFpRehashSelected(bool): Not implemented yet!" );
}

#include "SysCfgSettingsWidgetData.moc"
