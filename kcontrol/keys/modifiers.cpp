#include "modifiers.h"

#include <qlabel.h>
#include <qlayout.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <ctype.h>
#undef NONE

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
	QGridLayout* pLayoutTop = new QGridLayout( this, 4, 2, KDialog::marginHint() );
	pLayoutTop->setColStretch( 1, 1 );
	pLayoutTop->setRowStretch( 3, 1 );

	I18N_NOOP("KDE Modifier Labels");

	QGridLayout* grid = new QGridLayout( this, 5, 3, KDialog::marginHint(), KDialog::spacingHint() );
	grid->setColStretch( 0, 1 );
	grid->setColStretch( 1, 1 );
	grid->setColStretch( 2, 1 );
	pLayoutTop->addLayout( grid, 0, 0 );

	grid->addWidget( new QLabel( i18n("Modifier"), this ), 0, 0 );
	grid->addWidget( new QLabel( i18n("X11-Mod"), this ), 0, 1 );
	grid->addWidget( new QLabel( i18n("Label"), this ), 0, 2 );

	grid->addWidget( new QLabel( i18n("QAccel", "Shift"), this ), 1, 0 );
	grid->addWidget( new QLabel( i18n("QAccel", "Shift"), this ), 1, 1 );
	m_pcbShift = new KComboBox( this );
	m_pcbShift->insertItem( i18n("QAccel", "Shift") );
	grid->addWidget( m_pcbShift, 1, 2 );

	grid->addWidget( new QLabel( i18n("QAccel", "Ctrl"), this ), 2, 0 );
	grid->addWidget( new QLabel( i18n("Control"), this ), 2, 1 );
	m_pcbCtrl = new KComboBox( this );
	m_pcbCtrl->insertItem( i18n("QAccel", "Ctrl") );
	//m_pcbCtrl->insertItem( i18n("Option") );
	grid->addWidget( m_pcbCtrl, 2, 2 );

	grid->addWidget( new QLabel( i18n("QAccel", "Alt"), this ), 3, 0 );
	grid->addWidget( new QLabel( "Mod1", this ), 3, 1 );
	m_pcbAlt = new KComboBox( this );
	m_pcbAlt->insertItem( i18n("QAccel", "Alt") );
	m_pcbAlt->insertItem( i18n("Command") );
	grid->addWidget( m_pcbAlt, 3, 2 );

	grid->addWidget( new QLabel( i18n("Win"), this ), 4, 0 );
	m_pcbWinX = newModXComboBox();
	int i;
	switch( KKeyNative::modX(KKey::WIN) ) {
		case Mod2Mask: i = 1; break;
		case Mod3Mask: i = 2; break;
		case Mod4Mask: i = 3; break;
		case Mod5Mask: i = 5; break;
		default:       i = 0;
	}
	m_pcbWinX->setCurrentItem( i );

	grid->addWidget( m_pcbWinX, 4, 1 );
	m_pcbWin = new KComboBox( this );
	m_pcbWin->insertItem( i18n("Win") );
	m_pcbWin->insertItem( i18n("Option") );
	m_pcbWin->insertItem( i18n("Meta") );
	m_pcbWin->insertItem( i18n("Super") );
	m_pcbWin->insertItem( i18n("Hyper") );
	grid->addWidget( m_pcbWin, 4, 2 );

	//------------------
	pLayoutTop->addRowSpacing( 1, KDialog::spacingHint() * 3 );

	grid = new QGridLayout( this, 9, 2, KDialog::marginHint(), 0 );
	pLayoutTop->addLayout( grid, 2, 0 );

	I18N_NOOP("X Modifier Mapping");

	grid->addWidget( new QLabel( i18n("X11-Mod"), this ), 0, 0 );
	grid->addWidget( new QLabel( i18n("Keys"), this ), 0, 1 );

	grid->addWidget( new QLabel( "shift", this ), 1, 0 );
	grid->addWidget( new QLabel( "lock", this ), 2, 0 );
	grid->addWidget( new QLabel( "control", this ), 3, 0 );
	grid->addWidget( new QLabel( "mod1", this ), 4, 0 );
	grid->addWidget( new QLabel( "mod2", this ), 5, 0 );
	grid->addWidget( new QLabel( "mod3", this ), 6, 0 );
	grid->addWidget( new QLabel( "mod4", this ), 7, 0 );
	grid->addWidget( new QLabel( "mod5", this ), 8, 0 );

	XModifierKeymap* xmk = XGetModifierMapping( qt_xdisplay() );

	for( int iMod = 0; iMod < 8; iMod++ ) {
		QString s;
		for( int iKey = 0; iKey < xmk->max_keypermod; iKey++ ) {
			uint symX = XKeycodeToKeysym( qt_xdisplay(), xmk->modifiermap[xmk->max_keypermod * iMod + iKey], 0 );
			if( symX && !s.isEmpty() )
				s += ", ";
			s += XKeysymToString( symX );
		}
		grid->addWidget( new QLabel( s, this ), iMod+1, 1 );
	}

	XFreeModifiermap(xmk);

}

KComboBox* ModifiersModule::newModXComboBox()
{
	KComboBox* pcb = new KComboBox( this );
	pcb->insertItem( "" );
	pcb->insertItem( "Mod2" );
	pcb->insertItem( "Mod3" );
	pcb->insertItem( "Mod4" );
	pcb->insertItem( "Mod5" );
	return pcb;
}

#include "modifiers.moc"
