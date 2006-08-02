/*****************************************************************

Copyright (c) 2001-2004 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef NOSLOTS
# define DEF( name, key, target, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(key); \
   connect(a, SIGNAL(triggered(bool)), target, SLOT(fnSlot))
#else
# define DEF( name, key, target, fnSlot ) \
   a = new KAction( i18n(name), actionCollection, name ); \
   a->setGlobalShortcut(key);
#endif

#ifdef KICKER_ALL_BINDINGS
#define LAUNCH_MENU
#define SHOW_DESKTOP
#endif

#ifdef LAUNCH_MENU
	new KAction(i18n("Panel"), actionCollection, "Program:kicker");
	DEF(I18N_NOOP("Popup Launch Menu" ), Qt::ALT+Qt::Key_F1,
                      MenuManager::self(), kmenuAccelActivated());
#endif

#ifdef SHOW_DESKTOP
	DEF(I18N_NOOP( "Toggle Showing Desktop" ), Qt::ALT+Qt::CTRL+Qt::Key_D,
            this, slotToggleShowDesktop());
#endif

#undef DEF
#undef WIN
