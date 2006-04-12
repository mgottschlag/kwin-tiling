/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qapplication.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <QVBoxLayout>
#include <QDesktopWidget>

#include <kcmoduleloader.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "display.h"

typedef KGenericFactory<KCMDisplay, QWidget> DisplayFactory;
K_EXPORT_COMPONENT_FACTORY ( kcm_display, DisplayFactory( "display" ) )

KCMDisplay::KCMDisplay( QWidget *parent, const char *name, const QStringList& )
    : KCModule( DisplayFactory::instance(), parent )
    , m_changed(false)
{
  m_tabs = new QTabWidget( this );

  addTab( "randr", i18n( "Size && Orientation" ) );
  addTab( "nvidiadisplay", i18n( "Graphics Adaptor" ) );
  addTab( "nvidia3d", i18n( "3D Options" ) );
  addTab( "kgamma", i18n( "Monitor Gamma" ) );
  if ( QApplication::desktop()->isVirtualDesktop() )
    addTab( "xinerama", i18n( "Multiple Monitors" ) );
  addTab( "energy", i18n( "Power Control" ) );

  QVBoxLayout *top = new QVBoxLayout( this );
  top->setMargin( 0 );
  top->setSpacing( KDialog::spacingHint() );
  top->addWidget( m_tabs );

  setButtons( Apply|Help );
  load();
}

void KCMDisplay::addTab( const QString &name, const QString &label )
{
  QWidget *page = new QWidget( m_tabs, name.latin1() );
  QVBoxLayout *top = new QVBoxLayout( page );
  top->setMargin( KDialog::marginHint() );

  KCModule *kcm = KCModuleLoader::loadModule( name, KCModuleLoader::None,page );

  if ( kcm )
  {
    top->addWidget( kcm );
    m_tabs->addTab( page, label );

    connect( kcm, SIGNAL( changed(bool) ), SLOT( moduleChanged(bool) ) );
    m_modules.insert(kcm, false);
  }
  else
    delete page;
}

void KCMDisplay::load()
{
  for (QMap<KCModule*, bool>::ConstIterator it = m_modules.begin(); it != m_modules.end(); ++it)
    it.key()->load();
}

void KCMDisplay::save()
{
  for (QMap<KCModule*, bool>::Iterator it = m_modules.begin(); it != m_modules.end(); ++it)
    if (it.value())
      it.key()->save();
}

void KCMDisplay::moduleChanged( bool isChanged )
{
  QMap<KCModule*, bool>::Iterator currentModule = m_modules.find(static_cast<KCModule*>(const_cast<QObject*>(sender())));
  Q_ASSERT(currentModule != m_modules.end());
  if (currentModule.value() == isChanged)
    return;

  currentModule.value() = isChanged;

  bool c = false;

  for (QMap<KCModule*, bool>::ConstIterator it = m_modules.begin(); it != m_modules.end(); ++it) {
    if (it.value()) {
      c = true;
      break;
    }
  }

  if (m_changed != c) {
    m_changed = c;
    emit changed(c);
  }
}

#include "display.moc"
