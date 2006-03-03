/* kasaboutdlg.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/

#include <qcheckbox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QFrame>

#include <kdeversion.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <ktextbrowser.h>

#ifdef USE_KSPY
#include <kspy.h>
#endif

#include "kasbar.h"
#include "kasitem.h"
#include "kaspopup.h"

#include "kasclockitem.h"
#include "kasloaditem.h"

#include "kasaboutdlg.h"
#include "version.h"

#define Icon(x) KGlobal::iconLoader()->loadIcon( x, KIcon::NoGroup, KIcon::SizeMedium )
#define LargeIcon(x) KGlobal::iconLoader()->loadIcon( x, KIcon::NoGroup, KIcon::SizeLarge )

KasAboutDialog::KasAboutDialog( QWidget *parent )
   : KDialogBase( KDialogBase::IconList, i18n("About Kasbar"),
		  KDialogBase::Ok,
		  KDialogBase::Ok,
		  parent, "kasbarAboutDialog", false )
{
#ifdef USE_KSPY
  KSpy::invoke();
#endif

  addInfoPage();
  addAuthorsPage();
  addBSDPage();
  addGPLPage();

  addDemoBar();

  resize( 760, 450 );
}

KasAboutDialog::~KasAboutDialog()
{

}

void KasAboutDialog::addDemoBar()
{
   KHBox *box = new KHBox( this );

   box->setLineWidth(2);
   box->setFrameStyle( QFrame::Panel | QFrame::Sunken );

   box->setSpacing( spacingHint() );
   box->setMargin( marginHint() );

   KasBar *bar = new KasBar( Qt::Horizontal, box );
   bar->setItemSize( KasBar::Large );
   bar->setMasked( false );

   KasItem *ci = new KasItem( bar );
   ci->setIcon( LargeIcon( "icons" ) );
   bar->append( ci );

   KasPopup *pop = new KasPopup( ci );
   ci->setPopup( pop );
   ci->setCustomPopup( true );
   connect( ci, SIGNAL(leftButtonClicked(QMouseEvent *)), ci, SLOT(togglePopup()) );

   KasBar *groupbar = bar->createChildBar( ( bar->orientation() == Qt::Horizontal ) ? Qt::Vertical : Qt::Horizontal, pop );
   KasItem *i = 0;

   KasClockItem *clk = new KasClockItem( groupbar );
   groupbar->append( clk );

   i = new KasLoadItem( groupbar );
   groupbar->append( i );

   groupbar->addTestItems();

   pop->resize( groupbar->size() );

   bar->setFixedSize( bar->itemExtent(), bar->itemExtent() );
   addWidgetBelowList( box );
}

void KasAboutDialog::addInfoPage()
{
   KVBox *aboutPage = addVBoxPage( i18n("About"), i18n("About Kasbar"), Icon( "appearance" ) );
   aboutPage->setSpacing( spacingHint() );

   new QLabel( i18n( "<qt><body>"
		     "<h2>Kasbar Version: %1</h2>"
		     "<b>KDE Version:</b> %2"
		     "</body></qt>" )
	       .arg( VERSION_STRING ).arg( KDE_VERSION_STRING ),
	       aboutPage );

   KTextBrowser *text5 = new KTextBrowser( aboutPage );
   text5->setText( i18n( "<html><body>"
			 "<p>Kasbar TNG began as a port of the original Kasbar applet to "
			 "the (then new) extension API, but ended up as a complete "
			 "rewrite because of the range of features needed by different "
			 "groups of users. In the process of the rewrite all the standard "
			 "features provided by the default taskbar were added, along with "
			 "some more original ones such as thumbnails."
			 "</p>"
			 "<p>"
			 "You can find information about the latest developments in Kasbar at "
			 "<a href=\"%3\">%4</a>, the Kasbar homepage."
			 "</p>"
			 "</body></html>" )
		   .arg( HOMEPAGE_URL ).arg( HOMEPAGE_URL ) );

   text5->setWordWrap( Q3TextEdit::WidgetWidth );
}

void KasAboutDialog::addAuthorsPage()
{
   KVBox *authorsPage = addVBoxPage( i18n("Authors"),
				     i18n("Kasbar Authors"), 
				     Icon( "kuser" ) );

   KTextBrowser *text = new KTextBrowser( authorsPage );
   text->setText( i18n(
     "<html>"

     "<b>Richard Moore</b> <a href=\"mailto:rich@kde.org\">rich@kde.org</a><br>"
     "<b>Homepage:</b> <a href=\"http://xmelegance.org/\">http://xmelegance.org/</a>"

     "<p>Developer and maintainer of the Kasbar TNG code.</p>"

     "<hr/>"

     "<b>Daniel M. Duley (Mosfet)</b> <a href=\"mailto:mosfet@kde.org\">mosfet@kde.org</a><br>"
     "<b>Homepage:</b> <a href=\"http://www.mosfet.org/\">http://www.mosfet.org/</a>"

     "<p>Mosfet wrote the original Kasbar applet on which this "
     "extension is based. There is little of the original code "
     "remaining, but the basic look in opaque mode is almost "
     "identical to this first implementation.</p>"

     "</html>" ) );

   text->setWordWrap( Q3TextEdit::WidgetWidth );
}

void KasAboutDialog::addBSDPage()
{
   KVBox *bsdLicense = addVBoxPage( i18n("BSD License"), QString(), Icon( "filefind" ) );

   new QLabel( i18n( "Kasbar may be used under the terms of either the BSD license, "
		     "or the GNU Public License." ), bsdLicense );

   KTextBrowser *text2 = new KTextBrowser( bsdLicense );
   text2->setText( "Some text of unsurpassed tediousness goes here." );
   text2->setWordWrap( Q3TextEdit::NoWrap );

   QString bsdFile = locate("data", "LICENSES/BSD");
   if ( !bsdFile.isEmpty() ) {
     QString result;
     QFile file( bsdFile );

     if ( file.open( QIODevice::ReadOnly ) )
     {
        QTextStream str(&file);
        result += str.read();
     }

     text2->setText( result );
   }
}

void KasAboutDialog::addGPLPage()
{
   KVBox *gplPage = addVBoxPage( i18n("GPL License"), QString(), Icon( "filefind" ) );

   new QLabel( i18n( "Kasbar may be used under the terms of either the BSD license, "
		     "or the GNU Public License." ), gplPage );

   KTextBrowser *text3 = new KTextBrowser( gplPage );
   text3->setText( "Some more text of unsurpassed tediousness goes here." );
   text3->setWordWrap( Q3TextEdit::NoWrap );

   QString gplFile = locate("data", "LICENSES/GPL_V2");
   if ( !gplFile.isEmpty() ) {
     QString result;
     QFile file( gplFile );

     if ( file.open( QIODevice::ReadOnly ) )
     {
        QTextStream str(&file);
        result += str.read();
     }

     text3->setText( result );
   }
}

#include "kasaboutdlg.moc"
