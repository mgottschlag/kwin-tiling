#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'InstUninstSettingsWidget.ui'
**
** Created: Tue Sep 18 12:15:16 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "InstUninstSettingsWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CInstUninstSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CInstUninstSettingsWidgetData::CInstUninstSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CInstUninstSettingsWidgetData" );
    resize( 314, 187 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setCaption( QT_KDE_I18N( "Form2", "" ) );
    CInstUninstSettingsWidgetDataLayout = new QGridLayout( this ); 
    CInstUninstSettingsWidgetDataLayout->setSpacing( 6 );
    CInstUninstSettingsWidgetDataLayout->setMargin( 11 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CInstUninstSettingsWidgetDataLayout->addItem( spacer, 3, 1 );

    GroupBox5 = new QGroupBox( this, "GroupBox5" );
    GroupBox5->setTitle( QT_KDE_I18N( "Upon Install:", "" ) );
    GroupBox5->setColumnLayout(0, Qt::Vertical );
    GroupBox5->layout()->setSpacing( 0 );
    GroupBox5->layout()->setMargin( 0 );
    GroupBox5Layout = new QGridLayout( GroupBox5->layout() );
    GroupBox5Layout->setAlignment( Qt::AlignTop );
    GroupBox5Layout->setSpacing( 6 );
    GroupBox5Layout->setMargin( 11 );

    itsFixTtfPsNamesUponInstall = new QCheckBox( GroupBox5, "itsFixTtfPsNamesUponInstall" );
    itsFixTtfPsNamesUponInstall->setText( QT_KDE_I18N( "Fix &TrueType Postscript names table", "" ) );
    QWhatsThis::add( itsFixTtfPsNamesUponInstall, QT_KDE_I18N( "Some TrueType fonts have incorrect Postscript names for certain characters.\n"
"\n"
"For example, \"Euro\" is sometimes listed as \"uni20ac\". This would affect any Postscript output from applications (and most produce Postscript when printing), as it would list the use of the \"Euro\" character -but when this Postscript output is sent to the printer, or viewed, the \"Euro\" symbol would not be found.\n"
"\n"
"Selecting this option will cause the installer to automatically correct any broken fonts.", "" ) );

    GroupBox5Layout->addWidget( itsFixTtfPsNamesUponInstall, 0, 0 );

    CInstUninstSettingsWidgetDataLayout->addMultiCellWidget( GroupBox5, 0, 0, 0, 1 );

    ButtonGroup1 = new QButtonGroup( this, "ButtonGroup1" );
    ButtonGroup1->setTitle( QT_KDE_I18N( "Uninstall - \"Move\" To Folder:", "" ) );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 6 );
    ButtonGroup1Layout->setMargin( 11 );

    itsUninstallDirButton = new QPushButton( ButtonGroup1, "itsUninstallDirButton" );
    itsUninstallDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsUninstallDirButton->sizePolicy().hasHeightForWidth() ) );
    itsUninstallDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsUninstallDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsUninstallDirButton->setText( QT_KDE_I18N( ".", "" ) );
    QToolTip::add( itsUninstallDirButton, QT_KDE_I18N( "Change Folder.", "" ) );

    ButtonGroup1Layout->addWidget( itsUninstallDirButton, 0, 3 );

    itsUninstallDirText = new QLabel( ButtonGroup1, "itsUninstallDirText" );
    itsUninstallDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsUninstallDirText->sizePolicy().hasHeightForWidth() ) );
    itsUninstallDirText->setFrameShape( QLabel::Panel );
    itsUninstallDirText->setFrameShadow( QLabel::Sunken );
    itsUninstallDirText->setText( QT_KDE_I18N( "TextLabel4", "" ) );
    itsUninstallDirText->setAlignment( int( QLabel::AlignAuto | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    ButtonGroup1Layout->addWidget( itsUninstallDirText, 0, 2 );

    CInstUninstSettingsWidgetDataLayout->addMultiCellWidget( ButtonGroup1, 2, 2, 0, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CInstUninstSettingsWidgetDataLayout->addItem( spacer_2, 1, 0 );

    // signals and slots connections
    connect( itsFixTtfPsNamesUponInstall, SIGNAL( toggled(bool) ), this, SLOT( fixTtfNamesSelected(bool) ) );
    connect( itsUninstallDirButton, SIGNAL( clicked() ), this, SLOT( uninstallDirButtonPressed() ) );

    // tab order
    setTabOrder( itsFixTtfPsNamesUponInstall, itsUninstallDirButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CInstUninstSettingsWidgetData::~CInstUninstSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CInstUninstSettingsWidgetData::fixTtfNamesSelected(bool)
{
    qWarning( "CInstUninstSettingsWidgetData::fixTtfNamesSelected(bool): Not implemented yet!" );
}

void CInstUninstSettingsWidgetData::moveToSelected(bool)
{
    qWarning( "CInstUninstSettingsWidgetData::moveToSelected(bool): Not implemented yet!" );
}

void CInstUninstSettingsWidgetData::processAfmsSelected(bool)
{
    qWarning( "CInstUninstSettingsWidgetData::processAfmsSelected(bool): Not implemented yet!" );
}

void CInstUninstSettingsWidgetData::uninstallDirButtonPressed()
{
    qWarning( "CInstUninstSettingsWidgetData::uninstallDirButtonPressed(): Not implemented yet!" );
}

#include "InstUninstSettingsWidgetData.moc"
