#ifndef NOSLOTS
# define DEF( name, key, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key)); \
   connect(a, SIGNAL(triggered(bool)), SLOT(fnSlot))
# define DEF2( name, key, receiver, slot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key)); \
   connect(a, SIGNAL(triggered(bool)), receiver, SLOT(fnSlot))
#else
# define DEF( name, key, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key));
# define DEF2( name, key, receiver, slot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key));
#endif

	new KAction( i18n("Desktop"), actionCollection, "Program:kdesktop" );

#ifndef NOSLOTS
	if (KAuthorized::authorizeKAction("run_command"))
	{
#endif
		DEF( I18N_NOOP("Run Command"), Qt::ALT+Qt::Key_F2, slotExecuteCommand() );
#ifndef NOSLOTS
	}
#endif
	DEF( I18N_NOOP("Show Taskmanager"), Qt::CTRL+Qt::Key_Escape, slotShowTaskManager() );
	DEF( I18N_NOOP("Show Window List"), Qt::ALT+Qt::Key_F5, slotShowWindowList() );
	DEF( I18N_NOOP("Switch User"), Qt::ALT+Qt::CTRL+Qt::Key_Insert, slotSwitchUser() );
#ifndef NOSLOTS
	if (KAuthorized::authorizeKAction("lock_screen"))
	{
#endif
		DEF2( I18N_NOOP("Lock Session"), Qt::ALT+Qt::CTRL+Qt::Key_L, KRootWm::self(), SLOT(slotLock()) );
#ifndef NOSLOTS
	}
	if (KAuthorized::authorizeKAction("logout"))
	{
#endif
		DEF( I18N_NOOP("Log Out"), Qt::ALT+Qt::CTRL+Qt::Key_Delete, slotLogout() );
		DEF( I18N_NOOP("Log Out Without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete, slotLogoutNoCnf() );
		DEF( I18N_NOOP("Halt without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown, slotHaltNoCnf() );
		DEF( I18N_NOOP("Reboot without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp, slotRebootNoCnf() );
#ifndef NOSLOTS
	}
#endif

#undef DEF
#undef DEF2
