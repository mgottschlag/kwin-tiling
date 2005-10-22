#ifndef NOSLOTS
# define DEF( name, key, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key, this, SLOT(fnSlot) )
#else
# define DEF( name, key, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key )
#endif

	keys->insert( "Program:kxkb", i18n("Keyboard") );
	DEF( I18N_NOOP("Switch to Next Keyboard Layout"), Qt::ALT+Qt::CTRL+Qt::Key_K, toggled() );

#undef DEF
