#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'FontsWidget.ui'
**
** Created: Wed Oct 24 21:21:38 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "FontsWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <kprogress.h>
#include <qpushbutton.h>
#include <qsplitter.h>
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
    resize( 552, 314 ); 
    setCaption( tr2i18n( "Form1" ) );
    CFontsWidgetDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CFontsWidgetDataLayout"); 

    itsBox = new QGroupBox( this, "itsBox" );
    itsBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, itsBox->sizePolicy().hasHeightForWidth() ) );
    itsBox->setTitle( tr2i18n( "Preview:" ) );
    itsBox->setColumnLayout(0, Qt::Vertical );
    itsBox->layout()->setSpacing( 6 );
    itsBox->layout()->setMargin( 6 );
    itsBoxLayout = new QGridLayout( itsBox->layout() );
    itsBoxLayout->setAlignment( Qt::AlignTop );

    itsBackground = new QFrame( itsBox, "itsBackground" );
    QPalette pal;
    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, QColor( 195, 195, 195) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, QColor( 225, 225, 225) );
    cg.setColor( QColorGroup::Dark, QColor( 97, 97, 97) );
    cg.setColor( QColorGroup::Mid, QColor( 130, 130, 130) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, QColor( 249, 249, 249) );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, QColor( 168, 190, 218) );
    cg.setColor( QColorGroup::Background, white );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, black );
    cg.setColor( QColorGroup::LinkVisited, black );
    pal.setActive( cg );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, QColor( 195, 195, 195) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, QColor( 224, 224, 224) );
    cg.setColor( QColorGroup::Dark, QColor( 97, 97, 97) );
    cg.setColor( QColorGroup::Mid, QColor( 130, 130, 130) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, QColor( 249, 249, 249) );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, QColor( 168, 190, 218) );
    cg.setColor( QColorGroup::Background, white );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, black );
    cg.setColor( QColorGroup::LinkVisited, black );
    pal.setInactive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Button, QColor( 195, 195, 195) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, QColor( 224, 224, 224) );
    cg.setColor( QColorGroup::Dark, QColor( 97, 97, 97) );
    cg.setColor( QColorGroup::Mid, QColor( 130, 130, 130) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, QColor( 249, 249, 249) );
    cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Base, QColor( 168, 190, 218) );
    cg.setColor( QColorGroup::Background, white );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, black );
    cg.setColor( QColorGroup::LinkVisited, black );
    pal.setDisabled( cg );
    itsBackground->setPalette( pal );
    itsBackground->setFrameShape( QFrame::WinPanel );
    itsBackground->setFrameShadow( QFrame::Sunken );
    itsBackgroundLayout = new QGridLayout( itsBackground, 1, 1, 2, 0, "itsBackgroundLayout"); 

    itsProgress = new KProgress( itsBackground, "itsProgress" );
    itsProgress->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, itsProgress->sizePolicy().hasHeightForWidth() ) );
    itsProgress->setMinimumSize( QSize( 160, 0 ) );
    itsProgress->setMaximumSize( QSize( 160, 32767 ) );

    itsBackgroundLayout->addWidget( itsProgress, 0, 3 );

    itsLabel = new QLabel( itsBackground, "itsLabel" );
    itsLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, itsLabel->sizePolicy().hasHeightForWidth() ) );
    itsLabel->setMinimumSize( QSize( 0, 32 ) );
    itsLabel->setText( tr2i18n( "No preview available" ) );

    itsBackgroundLayout->addWidget( itsLabel, 0, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    itsBackgroundLayout->addItem( spacer, 0, 2 );

    itsBoxLayout->addWidget( itsBackground, 0, 0 );

    CFontsWidgetDataLayout->addWidget( itsBox, 1, 0 );

    itsSplitter = new QSplitter( this, "itsSplitter" );
    itsSplitter->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, itsSplitter->sizePolicy().hasHeightForWidth() ) );

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
