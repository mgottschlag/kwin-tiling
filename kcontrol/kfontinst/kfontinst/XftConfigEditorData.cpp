#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'XftConfigEditor.ui'
**
** Created: Wed Nov 21 00:35:22 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "XftConfigEditorData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CXftConfigEditorData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CXftConfigEditorData::CXftConfigEditorData( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : KDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CXftConfigEditorData" );
    resize( 553, 297 ); 
    setCaption( tr2i18n( "XftConfig Rule Editor" ) );
    CXftConfigEditorDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CXftConfigEditorDataLayout"); 

    GroupBox10 = new QGroupBox( this, "GroupBox10" );
    GroupBox10->setTitle( tr2i18n( "Match:" ) );
    GroupBox10->setColumnLayout(0, Qt::Vertical );
    GroupBox10->layout()->setSpacing( 6 );
    GroupBox10->layout()->setMargin( 11 );
    GroupBox10Layout = new QGridLayout( GroupBox10->layout() );
    GroupBox10Layout->setAlignment( Qt::AlignTop );

    Frame10 = new QFrame( GroupBox10, "Frame10" );
    Frame10->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)5, 0, 0, Frame10->sizePolicy().hasHeightForWidth() ) );
    Frame10->setFrameShape( QFrame::NoFrame );
    Frame10->setFrameShadow( QFrame::Raised );
    Frame10Layout = new QGridLayout( Frame10, 1, 1, 0, 6, "Frame10Layout"); 

    itsMatchAddButton = new QPushButton( Frame10, "itsMatchAddButton" );
    itsMatchAddButton->setText( tr2i18n( "&Add" ) );

    Frame10Layout->addWidget( itsMatchAddButton, 0, 0 );

    itsMatchRemoveButton = new QPushButton( Frame10, "itsMatchRemoveButton" );
    itsMatchRemoveButton->setText( tr2i18n( "&Remove" ) );

    Frame10Layout->addWidget( itsMatchRemoveButton, 1, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Frame10Layout->addItem( spacer, 2, 0 );

    GroupBox10Layout->addWidget( Frame10, 0, 2 );

    itsMatchList = new QListBox( GroupBox10, "itsMatchList" );

    GroupBox10Layout->addMultiCellWidget( itsMatchList, 0, 0, 0, 1 );

    Frame5 = new QFrame( GroupBox10, "Frame5" );
    Frame5->setFrameShape( QFrame::NoFrame );
    Frame5->setFrameShadow( QFrame::Raised );
    Frame5Layout = new QGridLayout( Frame5, 1, 1, 0, 6, "Frame5Layout"); 

    itsMatchQualCombo = new QComboBox( FALSE, Frame5, "itsMatchQualCombo" );
    itsMatchQualCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsMatchQualCombo->sizePolicy().hasHeightForWidth() ) );

    Frame5Layout->addWidget( itsMatchQualCombo, 0, 0 );

    itsMatchFieldNameCombo = new QComboBox( FALSE, Frame5, "itsMatchFieldNameCombo" );
    itsMatchFieldNameCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsMatchFieldNameCombo->sizePolicy().hasHeightForWidth() ) );

    Frame5Layout->addWidget( itsMatchFieldNameCombo, 0, 1 );

    itsMatchCompareCombo = new QComboBox( FALSE, Frame5, "itsMatchCompareCombo" );
    itsMatchCompareCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsMatchCompareCombo->sizePolicy().hasHeightForWidth() ) );

    Frame5Layout->addWidget( itsMatchCompareCombo, 0, 2 );

    GroupBox10Layout->addWidget( Frame5, 1, 0 );

    Frame7 = new QFrame( GroupBox10, "Frame7" );
    Frame7->setFrameShape( QFrame::NoFrame );
    Frame7->setFrameShadow( QFrame::Raised );
    Frame7Layout = new QGridLayout( Frame7, 1, 1, 0, 0, "Frame7Layout"); 

    itsMatchString = new QLineEdit( Frame7, "itsMatchString" );
    itsMatchString->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsMatchString->sizePolicy().hasHeightForWidth() ) );

    Frame7Layout->addWidget( itsMatchString, 0, 1 );

    itsMatchCombo = new QComboBox( FALSE, Frame7, "itsMatchCombo" );
    itsMatchCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsMatchCombo->sizePolicy().hasHeightForWidth() ) );

    Frame7Layout->addWidget( itsMatchCombo, 0, 0 );

    itsMatchOther = new QLineEdit( Frame7, "itsMatchOther" );
    itsMatchOther->setEnabled( FALSE );

    Frame7Layout->addMultiCellWidget( itsMatchOther, 1, 1, 0, 1 );

    GroupBox10Layout->addMultiCellWidget( Frame7, 1, 1, 1, 2 );

    CXftConfigEditorDataLayout->addMultiCellWidget( GroupBox10, 0, 0, 0, 2 );

    PushButton15 = new QPushButton( this, "PushButton15" );
    PushButton15->setText( tr2i18n( "&Cancel" ) );

    CXftConfigEditorDataLayout->addWidget( PushButton15, 2, 2 );

    itsOkButton = new QPushButton( this, "itsOkButton" );
    itsOkButton->setText( tr2i18n( "&OK" ) );

    CXftConfigEditorDataLayout->addWidget( itsOkButton, 2, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    CXftConfigEditorDataLayout->addItem( spacer_2, 2, 0 );

    GroupBox10_2 = new QGroupBox( this, "GroupBox10_2" );
    GroupBox10_2->setTitle( tr2i18n( "Edit:" ) );
    GroupBox10_2->setColumnLayout(0, Qt::Vertical );
    GroupBox10_2->layout()->setSpacing( 6 );
    GroupBox10_2->layout()->setMargin( 11 );
    GroupBox10_2Layout = new QGridLayout( GroupBox10_2->layout() );
    GroupBox10_2Layout->setAlignment( Qt::AlignTop );

    Frame5_2 = new QFrame( GroupBox10_2, "Frame5_2" );
    Frame5_2->setFrameShape( QFrame::NoFrame );
    Frame5_2->setFrameShadow( QFrame::Raised );
    Frame5_2Layout = new QGridLayout( Frame5_2, 1, 1, 0, 6, "Frame5_2Layout"); 

    itsEditFieldNameCombo = new QComboBox( FALSE, Frame5_2, "itsEditFieldNameCombo" );
    itsEditFieldNameCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsEditFieldNameCombo->sizePolicy().hasHeightForWidth() ) );

    Frame5_2Layout->addWidget( itsEditFieldNameCombo, 0, 1 );

    itsEditAssignCombo = new QComboBox( FALSE, Frame5_2, "itsEditAssignCombo" );
    itsEditAssignCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsEditAssignCombo->sizePolicy().hasHeightForWidth() ) );

    Frame5_2Layout->addWidget( itsEditAssignCombo, 0, 2 );

    GroupBox10_2Layout->addWidget( Frame5_2, 0, 0 );

    Frame7_2 = new QFrame( GroupBox10_2, "Frame7_2" );
    Frame7_2->setFrameShape( QFrame::NoFrame );
    Frame7_2->setFrameShadow( QFrame::Raised );
    Frame7_2Layout = new QGridLayout( Frame7_2, 1, 1, 0, 0, "Frame7_2Layout"); 

    itsEditString = new QLineEdit( Frame7_2, "itsEditString" );
    itsEditString->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsEditString->sizePolicy().hasHeightForWidth() ) );

    Frame7_2Layout->addWidget( itsEditString, 0, 1 );

    itsEditCombo = new QComboBox( FALSE, Frame7_2, "itsEditCombo" );
    itsEditCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsEditCombo->sizePolicy().hasHeightForWidth() ) );

    Frame7_2Layout->addWidget( itsEditCombo, 0, 0 );

    itsEditOther = new QLineEdit( Frame7_2, "itsEditOther" );
    itsEditOther->setEnabled( FALSE );

    Frame7_2Layout->addMultiCellWidget( itsEditOther, 1, 1, 0, 1 );

    GroupBox10_2Layout->addWidget( Frame7_2, 0, 1 );

    CXftConfigEditorDataLayout->addMultiCellWidget( GroupBox10_2, 1, 1, 0, 2 );

    // signals and slots connections
    connect( itsMatchFieldNameCombo, SIGNAL( activated(const QString&) ), this, SLOT( matchFieldSelected(const QString&) ) );
    connect( itsOkButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButton15, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( itsEditFieldNameCombo, SIGNAL( activated(const QString&) ), this, SLOT( editFieldSelected(const QString&) ) );
    connect( itsEditCombo, SIGNAL( activated(const QString&) ), this, SLOT( editCombo(const QString&) ) );
    connect( itsMatchCombo, SIGNAL( activated(const QString&) ), this, SLOT( matchCombo(const QString&) ) );
    connect( itsMatchAddButton, SIGNAL( clicked() ), this, SLOT( addMatch() ) );
    connect( itsMatchRemoveButton, SIGNAL( clicked() ), this, SLOT( removeMatch() ) );
    connect( itsMatchList, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( matchSelected(QListBoxItem*) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CXftConfigEditorData::~CXftConfigEditorData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CXftConfigEditorData::addMatch()
{
    qWarning( "CXftConfigEditorData::addMatch(): Not implemented yet!" );
}

void CXftConfigEditorData::editCombo(const QString &)
{
    qWarning( "CXftConfigEditorData::editCombo(const QString &): Not implemented yet!" );
}

void CXftConfigEditorData::editFieldSelected(const QString &)
{
    qWarning( "CXftConfigEditorData::editFieldSelected(const QString &): Not implemented yet!" );
}

void CXftConfigEditorData::matchCombo(const QString &)
{
    qWarning( "CXftConfigEditorData::matchCombo(const QString &): Not implemented yet!" );
}

void CXftConfigEditorData::matchFieldSelected(const QString &)
{
    qWarning( "CXftConfigEditorData::matchFieldSelected(const QString &): Not implemented yet!" );
}

void CXftConfigEditorData::matchSelected(QListBoxItem *)
{
    qWarning( "CXftConfigEditorData::matchSelected(QListBoxItem *): Not implemented yet!" );
}

void CXftConfigEditorData::removeMatch()
{
    qWarning( "CXftConfigEditorData::removeMatch(): Not implemented yet!" );
}

#include "XftConfigEditorData.moc"
