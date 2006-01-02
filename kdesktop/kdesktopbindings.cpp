#ifndef NOSLOTS
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString(), key3, this, SLOT(fnSlot) )
# define DEF2( name, key3, key4, receiver, slot ) \
   keys->insert( name, i18n(name), QString(), key3, receiver, slot );
#else
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString(), key3 )
# define DEF2( name, key3, key4, receiver, slot ) \
   keys->insert( name, i18n(name), QString(), key3 )
#endif
#define WIN Qt::META

	keys->insert( "Program:kdesktop", i18n("Desktop") );

#ifndef NOSLOTS
	if (KAuthorized::authorizeKAction("run_command"))
	{
#endif
		DEF( I18N_NOOP("Run Command"), Qt::ALT+Qt::Key_F2, WIN+Qt::Key_Return, slotExecuteCommand() );
#ifndef NOSLOTS
	}
#endif
	DEF( I18N_NOOP("Show Taskmanager"), Qt::CTRL+Qt::Key_Escape, WIN+Qt::CTRL+Qt::Key_Pause, slotShowTaskManager() );
	DEF( I18N_NOOP("Show Window List"), Qt::ALT+Qt::Key_F5, WIN+Qt::Key_0, slotShowWINdowList() );
	DEF( I18N_NOOP("Switch User"), Qt::ALT+Qt::CTRL+Qt::Key_Insert, WIN+Qt::Key_Insert, slotSwitchUser() );
#ifndef NOSLOTS
	if (KAuthorized::authorizeKAction("lock_screen"))
	{
#endif
		DEF2( I18N_NOOP("Lock Session"), Qt::ALT+Qt::CTRL+Qt::Key_L, WIN+Qt::Key_ScrollLock, KRootWm::self(), SLOT(slotLock()) );
#ifndef NOSLOTS
	}
	if (KAuthorized::authorizeKAction("logout"))
	{
#endif
		DEF( I18N_NOOP("Log Out"), Qt::ALT+Qt::CTRL+Qt::Key_Delete, WIN+Qt::Key_Escape, slotLogout() );
		DEF( I18N_NOOP("Log Out Without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete, WIN+Qt::SHIFT+Qt::Key_Escape, slotLogoutNoCnf() );
		DEF( I18N_NOOP("Halt without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown, WIN+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown, slotHQt::ALTNoCnf() );
		DEF( I18N_NOOP("Reboot without Confirmation"), Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp, WIN+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp, slotRebootNoCnf() );
#ifndef NOSLOTS
	}
#endif

#undef DEF
#undef DEF2
#undef WIN
