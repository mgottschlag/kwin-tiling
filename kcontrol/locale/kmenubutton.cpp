/*
 * klanguagebutton.cpp - Adds some methods for inserting languages.
 *
 * Copyright (c) 1999-2001 Hans Petter Bieker <bieker@kde.org>
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

#include "kmenubutton.h"
#include "kmenubutton.moc"

#include <kdebug.h>

static inline void checkInsertPos( QPopupMenu *popup, const QString & str,
                                   int &index )
{
  if ( index == -1 )
    return;

  int a = 0;
  int b = popup->count();
  while ( a < b )
  {
    int w = ( a + b ) / 2;

    int id = popup->idAt( w );
    int j = str.compare( popup->text( id ) );

    if ( j > 0 )
      a = w + 1;
    else
      b = w;
  }

  index = a; // it doesn't really matter ... a == b here.

  ASSERT( a == b );
}

static inline QPopupMenu * checkInsertIndex( QPopupMenu *popup,
                            const QStringList *tags, const QString &submenu )
{
  int pos = tags->findIndex( submenu );

  QPopupMenu *pi = 0;
  if ( pos != -1 )
  {
    QMenuItem *p = popup->findItem( pos );
    pi = p ? p->popup() : 0;
  }
  if ( !pi )
    pi = popup;

  return pi;
}


KMenuButton::~KMenuButton()
{
}

KMenuButton::KMenuButton( QWidget * parent, const char *name )
: QPushButton( parent, name ),
	m_popup( 0 ),
	m_oldPopup( 0 )
{
  m_tags = new QStringList;

  clear();
}

void KMenuButton::insertItem( const QIconSet& icon, const QString &text,
                      const QString &tag, const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, count(), index );
  m_tags->append( tag );
}

void KMenuButton::insertItem( const QString &text, const QString &tag,
                                  const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( text, count(), index );
  m_tags->append( tag );
}

void KMenuButton::insertSeparator( const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  pi->insertSeparator( index );
  m_tags->append( QString::null );
}

void KMenuButton::insertSubmenu( const QString &text, const QString &tag,
                                     const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  QPopupMenu *p = new QPopupMenu( pi );
  checkInsertPos( pi, text, index );
  pi->insertItem( text, p, count(), index );
  m_tags->append( tag );
  connect( p, SIGNAL( activated( int ) ),
                        SLOT( slotActivated( int ) ) );
  connect( p, SIGNAL( highlighted( int ) ), this,
                        SIGNAL( highlighted( int ) ) );
}

void KMenuButton::slotActivated( int index )
{
  // Update caption and iconset:
  if ( m_current == index )
    return;

  setCurrentItem( index );

  // Forward event from popup menu as if it was emitted from this widget:
  emit activated( index );
}

int KMenuButton::count() const
{
  return m_tags->count();
}

void KMenuButton::clear()
{
  m_tags->clear();

  delete m_oldPopup;
  m_oldPopup = m_popup;
  m_popup = new QPopupMenu( this );

  setPopup( m_popup );

  connect( m_popup, SIGNAL( activated( int ) ),
                        SLOT( slotActivated( int ) ) );
  connect( m_popup, SIGNAL( highlighted( int ) ),
                        SIGNAL( highlighted( int ) ) );
}

bool KMenuButton::containsTag( const QString &str ) const
{
  return m_tags->contains( str ) > 0;
}

QString KMenuButton::currentTag() const
{
  return *m_tags->at( currentItem() );
}

QString KMenuButton::tag( int i ) const
{
  if ( i < 0 || i >= count() )
  {
    kdDebug() << "KMenuButton::tag(), unknown tag " << i << endl;
    return QString::null;
  }
  return *m_tags->at( i );
}

int KMenuButton::currentItem() const
{
  return m_current;
}

void KMenuButton::setCurrentItem( int i )
{
  if ( i < 0 || i >= count() )
    return;
  m_current = i;
}

void KMenuButton::setCurrentItem( const QString &code )
{
  int i = m_tags->findIndex( code );
  if ( code.isNull() )
    i = 0;
  if ( i != -1 )
    setCurrentItem( i );
}

