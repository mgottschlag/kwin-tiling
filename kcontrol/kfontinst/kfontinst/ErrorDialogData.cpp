#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'ErrorDialog.ui'
**
** Created: Wed Sep 26 18:43:56 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "ErrorDialogData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CErrorDialogData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CErrorDialogData::CErrorDialogData( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : KDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ErrorDialog" );
    resize( 297, 235 ); 
    setCaption( QT_KDE_I18N( "Errors", "" ) );
    setSizeGripEnabled( FALSE );
    ErrorDialogLayout = new QGridLayout( this ); 
    ErrorDialogLayout->setSpacing( 6 );
    ErrorDialogLayout->setMargin( 11 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setText( QT_KDE_I18N( "&OK", "" ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    ErrorDialogLayout->addLayout( Layout1, 1, 0 );

    itsGroupBox = new QGroupBox( this, "itsGroupBox" );
    itsGroupBox->setTitle( QT_KDE_I18N( "12345678901234567890123456789012345678901234567890", "" ) );
    itsGroupBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, itsGroupBox->sizePolicy().hasHeightForWidth() ) );
    itsGroupBox->setColumnLayout(0, Qt::Vertical );
    itsGroupBox->layout()->setSpacing( 0 );
    itsGroupBox->layout()->setMargin( 0 );
    itsGroupBoxLayout = new QGridLayout( itsGroupBox->layout() );
    itsGroupBoxLayout->setAlignment( Qt::AlignTop );
    itsGroupBoxLayout->setSpacing( 6 );
    itsGroupBoxLayout->setMargin( 11 );

    itsListView = new QListView( itsGroupBox, "itsListView" );
    itsListView->addColumn( QT_KDE_I18N( "Item", "" ) );
    itsListView->header()->setClickEnabled( FALSE, itsListView->header()->count() - 1 );
    itsListView->addColumn( QT_KDE_I18N( "Reason", "" ) );
    itsListView->header()->setClickEnabled( FALSE, itsListView->header()->count() - 1 );
    itsListView->setSelectionMode( QListView::NoSelection );
    QWhatsThis::add( itsListView, QT_KDE_I18N( "Lists any errors associated with a font file.", "" ) );

    itsGroupBoxLayout->addWidget( itsListView, 0, 0 );

    ErrorDialogLayout->addWidget( itsGroupBox, 0, 0 );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CErrorDialogData::~CErrorDialogData()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "ErrorDialogData.moc"
