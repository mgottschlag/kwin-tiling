/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include "konqy_menu.h"
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <ksimpleconfig.h>

#include <qregexp.h>
#include <qfileinfo.h>
#include <ktoolinvocation.h>

K_EXPORT_KICKER_MENUEXT(konqueror, KonquerorProfilesMenu)

KonquerorProfilesMenu::KonquerorProfilesMenu(QWidget *parent, const QStringList & /*args*/)
: KPanelMenu("", parent)
{
    static bool kdeprintIconsInitialized = false;
    if ( !kdeprintIconsInitialized ) {
        KGlobal::iconLoader()->addAppDir("kdeprint");
        kdeprintIconsInitialized = true;
    }
}

KonquerorProfilesMenu::~KonquerorProfilesMenu()
{
}

void KonquerorProfilesMenu::initialize()
{
   if (initialized()) clear();
   setInitialized(true);

   QStringList profiles = KGlobal::dirs()->findAllResources( "data", "konqueror/profiles/*", false, true );

   m_profiles.resize(profiles.count());
   int id=1;
   QStringList::ConstIterator pEnd = profiles.end();
   for (QStringList::ConstIterator pIt = profiles.begin(); pIt != pEnd; ++pIt )
   {
      QFileInfo info( *pIt );
      QString profileName = KIO::decodeFileName( info.baseName() );
      QString niceName=profileName;
      KSimpleConfig cfg( *pIt, true );
      if ( cfg.hasGroup( "Profile" ) )
      {
         cfg.setGroup( "Profile" );
         if ( cfg.hasKey( "Name" ) )
            niceName = cfg.readEntry( "Name" );

         insertItem(niceName, id);
         m_profiles[id-1]=profileName;
         id++;
      }
   }
}

void KonquerorProfilesMenu::slotExec(int id)
{
   QStringList args;
   args<<"--profile"<<m_profiles[id-1];
   KToolInvocation::kdeinitExec("konqueror", args);
}

void KonquerorProfilesMenu::reload()
{
   initialize();
}

void KonquerorProfilesMenu::slotAboutToShow()
{
    reinitialize();
    KPanelMenu::slotAboutToShow();
}


#include "konqy_menu.moc"

