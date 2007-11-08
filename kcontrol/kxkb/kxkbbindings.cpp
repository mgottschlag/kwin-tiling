# define DEF( name, key ) \
   QAction* a__ = actionCollection->addAction( name );   \
   a = qobject_cast<KAction*>(a__);                       \
   a->setText( i18n(name) );                              \
   qobject_cast<KAction*>( a) ->setGlobalShortcut(KShortcut(key));                  

    DEF( I18N_NOOP("Switch to Next Keyboard Layout"), Qt::ALT+Qt::CTRL+Qt::Key_K);

#undef DEF
