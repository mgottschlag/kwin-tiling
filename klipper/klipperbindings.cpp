#ifndef NOSLOTS
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4, this, SLOT(fnSlot) )
#else
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4 )
#endif
#define WIN KKey::QtWIN

	keys->insert( "Program:klipper", i18n("Clipboard") );

	DEF( I18N_NOOP("Show Klipper Popup-Menu"), ALT+CTRL+Qt::Key_V, WIN+CTRL+Qt::Key_V, slotPopupMenu() );
	DEF( I18N_NOOP("Manually Invoke Action on Current Clipboard"), ALT+CTRL+Qt::Key_R, WIN+CTRL+Qt::Key_R, slotRepeatAction() );
	DEF( I18N_NOOP("Enable/Disable Clipboard Actions"), ALT+CTRL+Qt::Key_X, WIN+CTRL+Qt::Key_X, toggleURLGrabber() );

#undef DEF
#undef WIN
