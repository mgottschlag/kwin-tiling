/*
 * klanguagebutton.cpp - Adds some methods for inserting languages.
 *
 * Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define INCLUDE_MENUITEM_DEF
#include <qpopupmenu.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "klanguagebutton.h"
#include "klanguagebutton.moc"
#include "helper.h"

#include <kdebug.h>

class KLanguageButtonPrivate
{
public:
  QPushButton * button;
};

KLanguageButton::KLanguageButton( QWidget * parent, const char *name )
  : QWidget( parent, name ),
    m_ids( new QStringList ),
    m_popup( 0 ),
    m_oldPopup( 0 ),
    d( new KLanguageButtonPrivate )
{
  QHBoxLayout *layout = new QHBoxLayout(this, 0, 0);
  layout->setAutoAdd(true);
  d->button = new QPushButton( this, name );

  clear();
}

KLanguageButton::~KLanguageButton()
{
  delete m_ids;

  delete d->button;
  delete d;
}

void KLanguageButton::insertItem( const QIconSet& icon, const QString &text,
                      const QString & id, const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, count(), index );
  m_ids->append( id );
}

void KLanguageButton::insertItem( const QString &text, const QString & id,
                                  const QString &submenu, int index )
{
  insertItem( QIconSet(), text, id, submenu, index );
}

void KLanguageButton::insertSeparator( const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  pi->insertSeparator( index );
  m_ids->append( QString::null );
}

void KLanguageButton::insertSubmenu( const QIconSet & icon,
				     const QString &text, const QString &id,
                                     const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  QPopupMenu *p = new QPopupMenu( pi );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, p, count(), index );
  m_ids->append( id );
  connect( p, SIGNAL( activated( int ) ),
                        SLOT( slotActivated( int ) ) );
  connect( p, SIGNAL( highlighted( int ) ), this,
                        SLOT( slotHighlighted( int ) ) );
}

void KLanguageButton::insertSubmenu( const QString &text, const QString &id,
                                     const QString &submenu, int index )
{
  insertSubmenu(QIconSet(), text, id, submenu, index);
}

void KLanguageButton::slotActivated( int index )
{
  //kdDebug() << "slotActivated" << index << endl;

  // Update caption and iconset:
  if ( m_current == index )
    return;

  setCurrentItem( index );

  // Forward event from popup menu as if it was emitted from this widget:
  QString id = *m_ids->at( index );
  emit activated( id );
}

void KLanguageButton::slotHighlighted( int index )
{
  //kdDebug() << "slotHighlighted" << index << endl;

  QString id = *m_ids->at( index );
  emit ( highlighted(id) );
}

int KLanguageButton::count() const
{
  return m_ids->count();
}

void KLanguageButton::clear()
{
  m_ids->clear();

  delete m_oldPopup;
  m_oldPopup = m_popup;
  m_popup = new QPopupMenu( this );

  d->button->setPopup( m_popup );

  connect( m_popup, SIGNAL( activated( int ) ),
                        SLOT( slotActivated( int ) ) );
  connect( m_popup, SIGNAL( highlighted( int ) ),
	   SLOT( slotHighlighted( int ) ) );

  d->button->setText( QString::null );
  d->button->setIconSet( QIconSet() );
}

bool KLanguageButton::contains( const QString & id ) const
{
  return m_ids->contains( id ) > 0;
}

QString KLanguageButton::current() const
{
  return *m_ids->at( currentItem() );
}

int KLanguageButton::currentItem() const
{
  return m_current;
}

void KLanguageButton::setCurrentItem( int i )
{
  if ( i < 0 || i >= count() )
    return;
  m_current = i;

  d->button->setText( m_popup->text( m_current ) );
  QIconSet *icon = m_popup->iconSet( m_current );
  if( icon )
    d->button->setIconSet( *icon );
  else
    d->button->setIconSet( QIconSet() );
}

void KLanguageButton::setCurrentItem( const QString & id )
{
  int i = m_ids->findIndex( id );
  if ( id.isNull() )
    i = 0;
  if ( i != -1 )
    setCurrentItem( i );
}
