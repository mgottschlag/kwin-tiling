#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'FontsWidget.ui'
**
** Created: Wed Nov 21 00:35:19 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "FontsWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qframe.h>
#include <qgroupbox.h>
#include <kprogress.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include "FontPreview.h"
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CFontsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CFontsWidgetData::CFontsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CFontsWidgetData" );
    resize( 419, 205 ); 
    setCaption( tr2i18n( "Form1" ) );
    CFontsWidgetDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CFontsWidgetDataLayout"); 

    itsBox = new QGroupBox( this, "itsBox" );
    itsBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsBox->sizePolicy().hasHeightForWidth() ) );
    itsBox->setTitle( tr2i18n( "Preview" ) );
    itsBox->setColumnLayout(0, Qt::Vertical );
    itsBox->layout()->setSpacing( 6 );
    itsBox->layout()->setMargin( 6 );
    itsBoxLayout = new QGridLayout( itsBox->layout() );
    itsBoxLayout->setAlignment( Qt::AlignTop );

    itsBackground = new QFrame( itsBox, "itsBackground" );
    itsBackground->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, itsBackground->sizePolicy().hasHeightForWidth() ) );
    itsBackground->setMinimumSize( QSize( 0, 0 ) );
    itsBackground->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    itsBackground->setFrameShape( QFrame::WinPanel );
    itsBackground->setFrameShadow( QFrame::Sunken );
    itsBackgroundLayout = new QGridLayout( itsBackground, 1, 1, 2, 2, "itsBackgroundLayout"); 

    itsProgress = new KProgress( itsBackground, "itsProgress" );
    itsProgress->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsProgress->sizePolicy().hasHeightForWidth() ) );
    itsProgress->setMinimumSize( QSize( 160, 0 ) );
    itsProgress->setMaximumSize( QSize( 160, 32767 ) );

    itsBackgroundLayout->addWidget( itsProgress, 0, 1 );

    itsLabel = new CFontPreview( itsBackground, "itsLabel" );
    itsLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, itsLabel->sizePolicy().hasHeightForWidth() ) );
    itsLabel->setMinimumSize( QSize( 0, 32 ) );

    itsBackgroundLayout->addWidget( itsLabel, 0, 0 );

    itsBoxLayout->addWidget( itsBackground, 0, 0 );

    CFontsWidgetDataLayout->addWidget( itsBox, 1, 0 );

    itsSplitter = new QSplitter( this, "itsSplitter" );
    itsSplitter->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, itsSplitter->sizePolicy().hasHeightForWidth() ) );

    CFontsWidgetDataLayout->addWidget( itsSplitter, 0, 0 );

    // signals and slots connections
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CFontsWidgetData::~CFontsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CFontsWidgetData::preview(const QString &)
{
    qWarning( "CFontsWidgetData::preview(const QString &): Not implemented yet!" );
}

#include "FontsWidgetData.moc"
