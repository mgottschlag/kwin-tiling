#ifndef NOSLOTS
# define DEF( name, key, fnSlot ) \
   a = actionCollection->addAction( name );                        \
   a->setText( i18n(name) );                                       \
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(key)); \
   connect(a, SIGNAL(triggered(bool)), SLOT(fnSlot))
#else
# define DEF( name, key, fnSlot ) \
   a = actionCollection->addAction( name );               \
   a->setText( i18n(name) );                              \
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(key));
#endif

        a = actionCollection->addAction( "Program:kxkb" );
        a->setText( i18n("Keyboard") );
	DEF( I18N_NOOP("Switch to Next Keyboard Layout"), Qt::ALT+Qt::CTRL+Qt::Key_K, toggled() );

#undef DEF
