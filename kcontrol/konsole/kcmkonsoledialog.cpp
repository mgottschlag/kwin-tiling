#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'kcmkonsoledialog.ui'
**
** Created: Sat Apr 28 22:22:32 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kcmkonsoledialog.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include "schemaeditor.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KCMKonsoleDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
KCMKonsoleDialog::KCMKonsoleDialog( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "KCMKonsoleDialog" );
    resize( 409, 440 ); 
    setCaption( i18n( "Form1" ) );
    KCMKonsoleDialogLayout = new QGridLayout( this ); 
    KCMKonsoleDialogLayout->setSpacing( 11 );
    KCMKonsoleDialogLayout->setMargin( 0 );

    TabWidget2 = new QTabWidget( this, "TabWidget2" );

    tab = new QWidget( TabWidget2, "tab" );
    tabLayout = new QGridLayout( tab ); 
    tabLayout->setSpacing( 6 );
    tabLayout->setMargin( 11 );

    ButtonGroup1 = new QButtonGroup( tab, "ButtonGroup1" );
    ButtonGroup1->setTitle( i18n( "Bars:" ) );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 6 );
    ButtonGroup1Layout->setMargin( 11 );
    QWhatsThis::add(ButtonGroup1, i18n("<b>Bars</b><p>Use this groupbox to control the various available scrollbars, toolbars, and menubar.</p>"));

    showMenuBarCB = new QCheckBox( ButtonGroup1, "showMenuBarCB" );
    showMenuBarCB->setText( i18n( "Show menubar" ) );

    ButtonGroup1Layout->addMultiCellWidget( showMenuBarCB, 0, 0, 0, 1 );

    showFrameCB = new QCheckBox( ButtonGroup1, "showFrameCB" );
    showFrameCB->setText( i18n( "Show frame" ) );

    ButtonGroup1Layout->addWidget( showFrameCB, 2, 0 );

    TextLabel1 = new QLabel( ButtonGroup1, "TextLabel1" );
    TextLabel1->setText( i18n( "Scrollbar position:" ) );

    ButtonGroup1Layout->addWidget( TextLabel1, 3, 0 );

    scrollBarCO = new QComboBox( FALSE, ButtonGroup1, "scrollBarCO" );
    scrollBarCO->insertItem( i18n( "Hide" ) );
    scrollBarCO->insertItem( i18n( "Left" ) );
    scrollBarCO->insertItem( i18n( "Right" ) );

    ButtonGroup1Layout->addWidget( scrollBarCO, 3, 1 );

    showToolBarCB = new QCheckBox( ButtonGroup1, "showToolBarCB" );
    showToolBarCB->setEnabled( FALSE );
    showToolBarCB->setText( i18n( "Show toolbar (session bar)" ) );

    ButtonGroup1Layout->addWidget( showToolBarCB, 1, 0 );

    tabLayout->addWidget( ButtonGroup1, 2, 0 );

    GroupBox1 = new QGroupBox( tab, "GroupBox1" );
    GroupBox1->setTitle( i18n( "Layout:" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );
    QWhatsThis::add(GroupBox1, i18n("<b>Layout</b><p>Use the options presented within this groupbox to change the appearance of konsole.</p>"));

    TextLabel5 = new QLabel( GroupBox1, "TextLabel5" );
    TextLabel5->setText( i18n( "Font:" ) );

    GroupBox1Layout->addWidget( TextLabel5, 0, 0 );

    fontCO = new QComboBox( FALSE, GroupBox1, "fontCO" );
    fontCO->insertItem( i18n( "Normal" ) );
    fontCO->insertItem( i18n( "Tiny" ) );
    fontCO->insertItem( i18n( "Small" ) );
    fontCO->insertItem( i18n( "Medium" ) );
    fontCO->insertItem( i18n( "Large" ) );
    fontCO->insertItem( i18n( "Huge" ) );
    fontCO->insertItem( i18n( "Linux" ) );
    fontCO->insertItem( i18n( "Unicode" ) );

    GroupBox1Layout->addWidget( fontCO, 0, 1 );

    fontPB = new QPushButton( GroupBox1, "fontPB" );
    fontPB->setText( i18n( "Custom..." ) );

    GroupBox1Layout->addWidget( fontPB, 0, 2 );

    fullScreenCB = new QCheckBox( GroupBox1, "fullScreenCB" );
    fullScreenCB->setText( i18n( "Full screen" ) );

    GroupBox1Layout->addMultiCellWidget( fullScreenCB, 1, 1, 0, 2 );

    tabLayout->addWidget( GroupBox1, 1, 0 );

    GroupBox2 = new QGroupBox( tab, "GroupBox2" );
    GroupBox2->setTitle( i18n( "Misc:" ) );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 0 );
    GroupBox2->layout()->setMargin( 0 );
    GroupBox2Layout = new QGridLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );
    GroupBox2Layout->setSpacing( 6 );
    GroupBox2Layout->setMargin( 11 );
    QWhatsThis::add(GroupBox2, i18n("<b>Misc</b><p>This groupbox contains general options for konsole that would not fit elsewhere.</p>"));

    historyCB = new QCheckBox( GroupBox2, "historyCB" );
    historyCB->setEnabled( FALSE );
    historyCB->setText( i18n( "History" ) );
    historyCB->setChecked( TRUE );

    GroupBox2Layout->addMultiCellWidget( historyCB, 4, 4, 0, 3 );

    warnCB = new QCheckBox( GroupBox2, "warnCB" );
    warnCB->setText( i18n( "Warn for Open Session on Quit" ) );

    GroupBox2Layout->addMultiCellWidget( warnCB, 3, 3, 0, 3 );

    TextLabel8 = new QLabel( GroupBox2, "TextLabel8" );
    TextLabel8->setText( i18n( "Codec:" ) );

    GroupBox2Layout->addMultiCellWidget( TextLabel8, 2, 2, 0, 1 );

    codecCO = new QComboBox( FALSE, GroupBox2, "codecCO" );
    codecCO->insertItem( i18n( "local" ) );

    GroupBox2Layout->addMultiCellWidget( codecCO, 2, 2, 2, 3 );

    terminalLE = new QLineEdit( GroupBox2, "terminalLE" );
    terminalLE->setEnabled( FALSE );
    terminalLE->setText( i18n( "xterm" ) );

    GroupBox2Layout->addWidget( terminalLE, 1, 3 );

    TextLabel1_2 = new QLabel( GroupBox2, "TextLabel1_2" );
    TextLabel1_2->setEnabled( FALSE );
    TextLabel1_2->setText( i18n( "Other terminal:" ) );

    GroupBox2Layout->addMultiCellWidget( TextLabel1_2, 1, 1, 1, 2 );

    terminalCB = new QCheckBox( GroupBox2, "terminalCB" );
    terminalCB->setText( i18n( "Do not use konsole as default terminal application" ) );
    QWhatsThis::add(terminalCB, i18n("<b>Do not use konsole as default terminal application</b><p>Select this option if you wish to use a terminal program such as xterm, eterm, or rxvt by default.</p>"));

    GroupBox2Layout->addMultiCellWidget( terminalCB, 0, 0, 0, 3 );
    QSpacerItem* spacer = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    GroupBox2Layout->addItem( spacer, 1, 0 );

    tabLayout->addWidget( GroupBox2, 0, 0 );
    TabWidget2->insertTab( tab, i18n( "&General" ) );

    tab_2 = new QWidget( TabWidget2, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2 ); 
    tabLayout_2->setSpacing( 6 );
    tabLayout_2->setMargin( 11 );

    SchemaEditor1 = new SchemaEditor( tab_2, "SchemaEditor1" );

    tabLayout_2->addWidget( SchemaEditor1, 0, 0 );
    TabWidget2->insertTab( tab_2, i18n( "&Schema" ) );

/*
    tab_3 = new QWidget( TabWidget2, "tab_3" );
    tabLayout_3 = new QGridLayout( tab_3 ); 
    tabLayout_3->setSpacing( 6 );
    tabLayout_3->setMargin( 11 );

    TextLabel9 = new QLabel( tab_3, "TextLabel9" );
    TextLabel9->setText( i18n( "Not yet implemented :-(" ) );

    tabLayout_3->addWidget( TextLabel9, 0, 0 );
    TabWidget2->insertTab( tab_3, i18n( "Sessions" ) );

    tab_4 = new QWidget( TabWidget2, "tab_4" );
    tabLayout_4 = new QGridLayout( tab_4 ); 
    tabLayout_4->setSpacing( 6 );
    tabLayout_4->setMargin( 11 );

    TextLabel10 = new QLabel( tab_4, "TextLabel10" );
    TextLabel10->setText( i18n( "Not yet implemented :-(" ) );

    tabLayout_4->addWidget( TextLabel10, 0, 0 );
    TabWidget2->insertTab( tab_4, i18n( "Keyboard" ) );
*/
    KCMKonsoleDialogLayout->addWidget( TabWidget2, 0, 0 );

    // signals and slots connections
    connect( terminalCB, SIGNAL( toggled(bool) ), terminalLE, SLOT( setEnabled(bool) ) );
    connect( terminalCB, SIGNAL( toggled(bool) ), TextLabel1_2, SLOT( setEnabled(bool) ) );

    // tab order
    setTabOrder( TabWidget2, terminalCB );
    setTabOrder( terminalCB, terminalLE );
    setTabOrder( terminalLE, codecCO );
    setTabOrder( codecCO, warnCB );
    setTabOrder( warnCB, historyCB );
    setTabOrder( historyCB, fontCO );
    setTabOrder( fontCO, fontPB );
    setTabOrder( fontPB, fullScreenCB );
    setTabOrder( fullScreenCB, showMenuBarCB );
    setTabOrder( showMenuBarCB, showToolBarCB );
    setTabOrder( showToolBarCB, showFrameCB );
    setTabOrder( showFrameCB, scrollBarCO );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KCMKonsoleDialog::~KCMKonsoleDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "kcmkonsoledialog.moc"
