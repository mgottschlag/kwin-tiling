#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'schemadialog.ui'
**
** Created: Sat Apr 28 22:22:25 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "schemadialog.h"

#include <kcolorbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a SchemaDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SchemaDialog::SchemaDialog( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "SchemaDialog" );
    resize( 525, 413 ); 
    setCaption( i18n( "Konsole schema editor" ) );
    SchemaDialogLayout = new QGridLayout( this ); 
    SchemaDialogLayout->setSpacing( 6 );
    SchemaDialogLayout->setMargin( 2 );

    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setTitle( i18n( "Schema" ) );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 0 );
    GroupBox2->layout()->setMargin( 0 );
    GroupBox2Layout = new QGridLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );
    GroupBox2Layout->setSpacing( 6 );
    GroupBox2Layout->setMargin( 11 );

    removeButton = new QPushButton( GroupBox2, "removeButton" );
    removeButton->setText( i18n( "&Remove schema" ) );

    GroupBox2Layout->addWidget( removeButton, 3, 0 );

    saveButton = new QPushButton( GroupBox2, "saveButton" );
    saveButton->setText( i18n( "&Save schema..." ) );

    GroupBox2Layout->addWidget( saveButton, 2, 0 );

    schemaList = new QListBox( GroupBox2, "schemaList" );

    GroupBox2Layout->addWidget( schemaList, 0, 0 );

    defaultSchemaCB = new QCheckBox( GroupBox2, "defaultSchemaCB" );
    defaultSchemaCB->setText( i18n( "Set as default schema" ) );

    GroupBox2Layout->addWidget( defaultSchemaCB, 1, 0 );

    SchemaDialogLayout->addMultiCellWidget( GroupBox2, 2, 2, 0, 1 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, GroupBox1->sizePolicy().hasHeightForWidth() ) );
    GroupBox1->setTitle( i18n( "Colors" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );

    colorCombo = new QComboBox( FALSE, GroupBox1, "colorCombo" );
    colorCombo->insertItem( i18n( "0 -Foreground color" ) );
    colorCombo->insertItem( i18n( "1 -Backgorund color" ) );
    colorCombo->insertItem( i18n( "2 - Color 0 (black)" ) );
    colorCombo->insertItem( i18n( "3 - Color 1 (red)" ) );
    colorCombo->insertItem( i18n( "4 - Color 2 (green)" ) );
    colorCombo->insertItem( i18n( "5 - Color 3 (yellow)" ) );
    colorCombo->insertItem( i18n( "6 - Color 4 (blue)" ) );
    colorCombo->insertItem( i18n( "7 - Color 5 (magenta)" ) );
    colorCombo->insertItem( i18n( "8 - Color 6 (cyan)" ) );
    colorCombo->insertItem( i18n( "9 - Color 7 (white)" ) );
    colorCombo->insertItem( i18n( "10 -Foreground intensive color" ) );
    colorCombo->insertItem( i18n( "11 - Background intensive color" ) );
    colorCombo->insertItem( i18n( "12 - Color 0 intensive (gray)" ) );
    colorCombo->insertItem( i18n( "13 - Color 1 intensive (light red)" ) );
    colorCombo->insertItem( i18n( "14 - Color 2 intensive (light green)" ) );
    colorCombo->insertItem( i18n( "15 - Color 3 intensive (light yellow)" ) );
    colorCombo->insertItem( i18n( "16 - Color 4 intensive (light blue)" ) );
    colorCombo->insertItem( i18n( "17 - Color 5 intensive (light magenta)" ) );
    colorCombo->insertItem( i18n( "18 - Color 6 intensive (light cyan)" ) );
    colorCombo->insertItem( i18n( "19 - Color 7 intensive (white)" ) );

    GroupBox1Layout->addWidget( colorCombo, 1, 0 );

    TextLabel8 = new QLabel( GroupBox1, "TextLabel8" );
    TextLabel8->setText( i18n( "Shell color:" ) );

    GroupBox1Layout->addWidget( TextLabel8, 0, 0 );

    boldCheck = new QCheckBox( GroupBox1, "boldCheck" );
    boldCheck->setText( i18n( "Bold" ) );

    GroupBox1Layout->addWidget( boldCheck, 2, 2 );

    transparentCheck = new QCheckBox( GroupBox1, "transparentCheck" );
    transparentCheck->setText( i18n( "Transparent" ) );

    GroupBox1Layout->addMultiCellWidget( transparentCheck, 2, 2, 3, 4 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    GroupBox1Layout->addItem( spacer, 1, 1 );

    colorButton = new KColorButton( GroupBox1, "colorButton" );
    colorButton->setText( QString::null );

    GroupBox1Layout->addWidget( colorButton, 1, 4 );

    typeCombo = new QComboBox( FALSE, GroupBox1, "typeCombo" );
    typeCombo->insertItem( i18n( "Custom" ) );
    typeCombo->insertItem( i18n( "System background" ) );
    typeCombo->insertItem( i18n( "System foreground" ) );

    GroupBox1Layout->addMultiCellWidget( typeCombo, 1, 1, 2, 3 );

    TextLabel1_2 = new QLabel( GroupBox1, "TextLabel1_2" );
    TextLabel1_2->setText( i18n( "Konsole color:" ) );

    GroupBox1Layout->addMultiCellWidget( TextLabel1_2, 0, 0, 2, 4 );

    SchemaDialogLayout->addMultiCellWidget( GroupBox1, 1, 1, 0, 2 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Title:" ) );

    SchemaDialogLayout->addWidget( TextLabel1, 0, 0 );

    titleLine = new QLineEdit( this, "titleLine" );

    SchemaDialogLayout->addMultiCellWidget( titleLine, 0, 0, 1, 2 );

    GroupBox13 = new QGroupBox( this, "GroupBox13" );
    GroupBox13->setTitle( i18n( "Backgorund" ) );
    GroupBox13->setColumnLayout(0, Qt::Vertical );
    GroupBox13->layout()->setSpacing( 0 );
    GroupBox13->layout()->setMargin( 0 );
    GroupBox13Layout = new QGridLayout( GroupBox13->layout() );
    GroupBox13Layout->setAlignment( Qt::AlignTop );
    GroupBox13Layout->setSpacing( 6 );
    GroupBox13Layout->setMargin( 11 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    GroupBox13Layout->addItem( spacer_2, 4, 0 );

    backgndLine = new QLineEdit( GroupBox13, "backgndLine" );

    GroupBox13Layout->addMultiCellWidget( backgndLine, 1, 1, 0, 4 );

    imageBrowse = new QToolButton( GroupBox13, "imageBrowse" );
    imageBrowse->setText( i18n( "..." ) );

    GroupBox13Layout->addWidget( imageBrowse, 1, 5 );

    modeCombo = new QComboBox( FALSE, GroupBox13, "modeCombo" );
    modeCombo->insertItem( i18n( "Tiled" ) );
    modeCombo->insertItem( i18n( "Centered" ) );
    modeCombo->insertItem( i18n( "Full" ) );

    GroupBox13Layout->addMultiCellWidget( modeCombo, 0, 0, 3, 4 );

    TextLabel11 = new QLabel( GroupBox13, "TextLabel11" );
    TextLabel11->setText( i18n( "Image" ) );

    GroupBox13Layout->addMultiCellWidget( TextLabel11, 0, 0, 0, 2 );

    TextLabel6 = new QLabel( GroupBox13, "TextLabel6" );
    TextLabel6->setText( i18n( "Min" ) );

    GroupBox13Layout->addWidget( TextLabel6, 5, 1 );

    shadeSlide = new QSlider( GroupBox13, "shadeSlide" );
    shadeSlide->setValue( 50 );
    shadeSlide->setOrientation( QSlider::Horizontal );

    GroupBox13Layout->addMultiCellWidget( shadeSlide, 5, 5, 2, 3 );

    TextLabel5 = new QLabel( GroupBox13, "TextLabel5" );
    TextLabel5->setText( i18n( "Max" ) );

    GroupBox13Layout->addWidget( TextLabel5, 5, 4 );

    TextLabel3 = new QLabel( GroupBox13, "TextLabel3" );
    TextLabel3->setText( i18n( "Shade to:" ) );

    GroupBox13Layout->addMultiCellWidget( TextLabel3, 4, 4, 1, 2 );

    shadeColor = new KColorButton( GroupBox13, "shadeColor" );
    shadeColor->setText( QString::null );

    GroupBox13Layout->addMultiCellWidget( shadeColor, 4, 4, 3, 4 );

    previewPixmap = new QLabel( GroupBox13, "previewPixmap" );
    previewPixmap->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, previewPixmap->sizePolicy().hasHeightForWidth() ) );
    previewPixmap->setMinimumSize( QSize( 180, 100 ) );
    previewPixmap->setMaximumSize( QSize( 180, 120 ) );
    previewPixmap->setFrameShape( QLabel::Panel );
    previewPixmap->setFrameShadow( QLabel::Sunken );
    previewPixmap->setScaledContents( TRUE );

    GroupBox13Layout->addMultiCellWidget( previewPixmap, 2, 2, 0, 4 );

    transparencyCheck = new QCheckBox( GroupBox13, "transparencyCheck" );
    transparencyCheck->setText( i18n( "Transparent" ) );

    GroupBox13Layout->addMultiCellWidget( transparencyCheck, 3, 3, 0, 5 );

    SchemaDialogLayout->addWidget( GroupBox13, 2, 2 );

    // signals and slots connections
    connect( transparencyCheck, SIGNAL( toggled(bool) ), shadeSlide, SLOT( setEnabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), shadeColor, SLOT( setEnabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), TextLabel3, SLOT( setEnabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), TextLabel6, SLOT( setEnabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), TextLabel5, SLOT( setDisabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), TextLabel5, SLOT( setEnabled(bool) ) );
    connect( defaultSchemaCB, SIGNAL( toggled(bool) ), defaultSchemaCB, SLOT( setDisabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), backgndLine, SLOT( setDisabled(bool) ) );
    connect( transparencyCheck, SIGNAL( toggled(bool) ), imageBrowse, SLOT( setDisabled(bool) ) );

    // tab order
    setTabOrder( titleLine, colorCombo );
    setTabOrder( colorCombo, typeCombo );
    setTabOrder( typeCombo, boldCheck );
    setTabOrder( boldCheck, transparentCheck );
    setTabOrder( transparentCheck, colorButton );
    setTabOrder( colorButton, schemaList );
    setTabOrder( schemaList, defaultSchemaCB );
    setTabOrder( defaultSchemaCB, saveButton );
    setTabOrder( saveButton, removeButton );
    setTabOrder( removeButton, modeCombo );
    setTabOrder( modeCombo, backgndLine );
    setTabOrder( backgndLine, transparencyCheck );
    setTabOrder( transparencyCheck, shadeColor );
    setTabOrder( shadeColor, shadeSlide );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SchemaDialog::~SchemaDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "schemadialog.moc"
