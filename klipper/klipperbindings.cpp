#ifndef NOSLOTS
# define DEF( name, key3, key4, fnSlot ) \
   keys->insertAction( name, i18n(name), QString::null, key3, key4, this, SLOT(fnSlot) )
#else
# define DEF( name, key3, key4, fnSlot ) \
   keys->insertAction( name, i18n(name), QString::null, key3, key4 )
#endif

	keys->insertLabel( "Program:klipper", i18n("Clipboard") );

	DEF( "Show Klipper Popup-Menu", "Alt+Ctrl+V", "Meta+Ctrl+V", slotPopupMenu() );
	DEF( "Manually Invoke Action on Current Clipboard", "Alt+Ctrl+R", "Meta+Ctrl+R", slotRepeatAction() );
	DEF( "Enable/Disable Clipboard Actions", "Alt+Ctrl+X", "Meta+Ctrl+X", toggleURLGrabber() );

#undef DEF
