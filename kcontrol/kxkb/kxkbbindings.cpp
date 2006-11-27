#ifndef NOSLOTS
# define DEF( name, key, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key)); \
   connect(a, SIGNAL(triggered(bool)), SLOT(fnSlot))
#else
# define DEF( name, key, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key));
#endif

	new KAction( i18n("Keyboard"), actionCollection, "Program:kxkb" );
	DEF( I18N_NOOP("Switch to Next Keyboard Layout"), Qt::ALT+Qt::CTRL+Qt::Key_K, toggled() );

#undef DEF
