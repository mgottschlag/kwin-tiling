#include "modifiers.h"

#include <qlabel.h>
#include <qlayout.h>

#include <kcombobox.h>
#include <kdialog.h>

/*Modifier Scheme
	PC: Shift/Ctrl/Alt/Win
	Mac: Shift/Command/Apple/Alt
	Custom

X11
Modifier	XMod		Label
Shift		Shift		[Shift]
Ctrl		Control		[Ctrl] Ctrl|Apple
Alt		[Mod1]		[Alt] Alt|Command
Win		[Mod4]		[Win] Win|Alt|Meta|Super|Hyper
Extra1		[]		[] User definable

	Shift
	Lock	Caps_Lock
	Control	Control_L, Control_R
	Mod1	Alt_L, Alt_R
	Mod2	Num_Lock
	Mod3	Mode_switch
	Mod4	Super_L, Super_R
	Mod5	Scroll_Lock
*/


ModifiersModule::ModifiersModule( QWidget *parent, const char *name )
:	QWidget( parent, name )
{
	QGridLayout *grid = new QGridLayout( this, 5, 3 );
	//grid->addRowSpacing( 0, 5 );

	grid->addWidget( new QLabel("Modifier", this), 0, 0 );
	grid->addWidget( new QLabel("X11-Mod", this), 0, 1 );
	grid->addWidget( new QLabel("Label", this), 0, 2 );

	grid->addWidget( new QLabel("Shift", this), 1, 0 );
	grid->addWidget( new QLabel("Shift", this), 1, 1 );
	m_pcbShift = new KComboBox( this );
	m_pcbShift->insertItem( "Shift" );
	grid->addWidget( m_pcbShift, 1, 2 );

	grid->addWidget( new QLabel("Ctrl", this), 2, 0 );
	grid->addWidget( new QLabel("Control", this), 2, 1 );
	m_pcbCtrl = new KComboBox( this );
	m_pcbCtrl->insertItem( "Ctrl" );
	m_pcbCtrl->insertItem( "Apple" );
	grid->addWidget( m_pcbCtrl, 2, 2 );

	grid->addWidget( new QLabel("Alt", this), 3, 0 );
	m_pcbAltX = newModXComboBox();
	grid->addWidget( m_pcbAltX, 3, 1 );
	m_pcbAlt = new KComboBox( this );
	m_pcbAlt->insertItem( "Alt" );
	m_pcbAlt->insertItem( "Command" );
	grid->addWidget( m_pcbAlt, 3, 2 );

	grid->addWidget( new QLabel("Win", this), 4, 0 );
	m_pcbWinX = newModXComboBox();
	grid->addWidget( m_pcbWinX, 4, 1 );
	m_pcbWin = new KComboBox( this );
	m_pcbWin->insertItem( "Win" );
	m_pcbWin->insertItem( "Alt" );
	m_pcbWin->insertItem( "Meta" );
	m_pcbWin->insertItem( "Super" );
	m_pcbWin->insertItem( "Hyper" );
	grid->addWidget( m_pcbWin, 4, 2 );
}

KComboBox* ModifiersModule::newModXComboBox()
{
	KComboBox* pcb = new KComboBox( this );
	pcb->insertItem( "Mod1" );
	pcb->insertItem( "Mod2" );
	pcb->insertItem( "Mod3" );
	pcb->insertItem( "Mod4" );
	pcb->insertItem( "Mod5" );
	return pcb;
}

#include "modifiers.moc"
