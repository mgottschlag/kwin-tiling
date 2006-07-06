#ifndef NOSLOTS
# define DEF( name, key3, key4, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key3, key4)); \
   connect(a, SIGNAL(triggered(bool)), SLOT(fnSlot))
# define DEF2( name, key3, key4, receiver, slot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key3, key4)); \
   connect(a, SIGNAL(triggered(bool)), receiver, SLOT(fnSlot))
#else
# define DEF( name, key3, key4, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key3, key4));
# define DEF2( name, key3, key4, receiver, slot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(KShortcut(key3, key4));
#endif

	new KAction( i18n("Desktop"), actionCollection, "Program:kdesktop" );

#ifndef NOSLOTS
	if (KAuthorized::authorizeKAction("run_command"))
	{
#endif
		DEF( I18N_NOOP("Run Command"), Qt::ALT+Qt::Key_F2, Qt::META+Qt::Key_Return, slotExecuteCommand() );
#ifndef NOSLOTS
	}
#endif
	DEF( I18N_NOOP("Show Taskmanager"), Qt::CTRL+Qt::Key_Escape, Qt::META+Qt::CTRL+Qt::Key_Pause, slotShowTaskManager() );
	DEF( I18N_NOOP("Show Window List"), Qt::ALT+Qt::Key_F5, Qt::META+Qt::Key_0, slotShowWindowList() );
	DEF( I18N_NOOP("Switch User"), Qt::ALT+Qt::CTRL+Qt::Key_Insert, Qt::META+Qt::Key_Insert, slotSwitchUser() );
#ifndef NOSLOTS
	if (KAuthorized::authorizeKAction("lock_screen"))
	{
#endif
		DEF2( I18N_NOOP("Lock Session"), Qt::ALT+Qt::CTRL+Qt::Key_L, Qt::META+Qt::Key_ScrollLock, KRootWm::self(), SLOT(slotLock()) );
#ifndef NOSLOTS
	}
	if (KAuthorized::authorizeKAction("logout"))
	{
#endif
		DEF( I18N_NOOP("Log Out"), Qt::ALT+Qt::CTRL+Qt::Key_Delete, Qt::META+Qt::Key_Escape, slotLogout() );
		DEF( I18N_NOOP("Log Out Without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete, Qt::META+Qt::SHIFT+Qt::Key_Escape, slotLogoutNoCnf() );
		DEF( I18N_NOOP("Halt without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown, Qt::META+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown, slotHaltNoCnf() );
		DEF( I18N_NOOP("Reboot without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp, Qt::META+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp, slotRebootNoCnf() );
#ifndef NOSLOTS
	}
#endif

#undef DEF
#undef DEF2
