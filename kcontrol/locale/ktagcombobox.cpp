/*
 * ktagcombobox.cpp - A combobox with support for submenues, icons and tags
 *
 * Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#define INCLUDE_MENUITEM_DEF 1
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qpopupmenu.h>
#include <qmenudata.h>

#include "ktagcombobox.h"
#include "ktagcombobox.moc"

KTagComboBox::~KTagComboBox ()
{
  delete popup;
  delete tags;
}

KTagComboBox::KTagComboBox (QWidget * parent, const char *name)
  : QWidget(parent, name)
{
  popup = new QPopupMenu;
  tags = new QStringList;
  connect( popup, SIGNAL(activated(int)),
                        SLOT(internalActivate(int)) );
  connect( popup, SIGNAL(highlighted(int)),
                        SLOT(internalHighlight(int)) );
  setFocusPolicy( TabFocus );
}

void KTagComboBox::popupMenu()
{
   popup->popup( mapToGlobal( QPoint(0,0) ), current );
}

void KTagComboBox::keyPressEvent( QKeyEvent *e )
{
    int c;

    if ( ( e->key() == Key_F4 && e->state() == 0 ) || 
         ( e->key() == Key_Down && (e->state() & AltButton) ) ||
         ( e->key() == Key_Space ) ) {
        if ( count() ) { 
            popup->setActiveItem( current );
            popupMenu();
        }
        return;
    } else {
        e->ignore();
        return;
    }

    c = currentItem();
    emit highlighted( c );
    emit activated( c );
}

void KTagComboBox::mousePressEvent( QMouseEvent * /*e*/ )
{
  popupMenu();
}

QSize KTagComboBox::sizeHint() const
{
  QString tmp;
  int i;
  QFontMetrics fm = fontMetrics();
  int maxH = QMAX(12, fm.height());
  int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;

  for(i = 0; i < count(); i++)
  {
    tmp = popup->text(i);
    int h = fm.width( tmp );
    if (h > maxW)
      maxW = h;
    h = fm.width( tmp );
    if (h > maxH)
      maxH = h;
  }

  maxW += 2*4 + 20;
  maxH += 2*5;
  return QSize (maxW, maxH);
}

void KTagComboBox::internalActivate( int index )
{
  if (current == index) return;
  current = index;
  emit activated( index );
  repaint();
}

void KTagComboBox::internalHighlight( int index )
{
  emit highlighted( index );
}

void KTagComboBox::clear()
{
  popup->clear();
  tags->clear();
}

int KTagComboBox::count() const
{
  return tags->count();
}

static inline QPopupMenu *checkInsertIndex(QPopupMenu *popup, const QStringList *tags, const QString &submenu, int *index)
{
  int pos = tags->findIndex(submenu);

  QPopupMenu *pi = 0;
  if (pos != -1)
  {
    QMenuItem *p = popup->findItem(popup->idAt(pos));
    pi = p?p->popup():0;
  }
  if (!pi) pi = popup;

  if (*index > (int)pi->count())
    *index = -1;

  return pi;
}

void KTagComboBox::insertItem(const QIconSet& icon, const QString &text, const QString &tag, const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex(popup, tags, submenu, &index);
  pi->insertItem(icon, text, count(), index);
  tags->append(tag);
}

void KTagComboBox::insertItem(const QString &text, const QString &tag, const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex(popup, tags, submenu, &index);
  pi->insertItem(text, count(), index);
  tags->append(tag);
}

void KTagComboBox::insertSeparator(const QString &submenu, int index)
{
  QPopupMenu *pi = checkInsertIndex(popup, tags, submenu, &index);
  pi->insertSeparator(index);
  tags->append(QString::null);
}

void KTagComboBox::insertSubmenu(const QString &text, const QString &tag, const QString &submenu, int index)
{
  QPopupMenu *pi = checkInsertIndex(popup, tags, submenu, &index);
  QPopupMenu *p = new QPopupMenu;
  pi->insertItem(text, p, count(), index);
  tags->append(tag);
  connect( p, SIGNAL(activated(int)),
                        SLOT(internalActivate(int)) );
  connect( p, SIGNAL(highlighted(int)),
                        SLOT(internalHighlight(int)) );
}

void KTagComboBox::changeItem( const QString &text, int index )
{
  popup->changeItem( text, index);
  if (index == current)
    repaint();
}

void KTagComboBox::paintEvent( QPaintEvent * )
{
  QPainter p (this);
  const QColorGroup & colorgr = colorGroup();
  QRect clip(2, 2, width() - 4, height() - 4);
  int dist, buttonH, buttonW;
  getMetrics( &dist, &buttonW, &buttonH );
  int posx = width() - (dist + buttonW + 1);

  // the largest box
  qDrawShadePanel( &p, rect(), colorgr, FALSE, style().defaultFrameWidth(),
                         &colorgr.brush( QColorGroup::Button ) );

  qDrawShadePanel( &p, posx, (height() - buttonH)/2,
                         buttonW, buttonH, colorgr, FALSE, style().defaultFrameWidth());
  // Text
  p.drawText(clip, AlignCenter | SingleLine, popup->text( current) );

  // Icon
  QIconSet *icon = popup->iconSet( this->current );
  if (icon) {
    QPixmap pm = icon->pixmap();
    p.drawPixmap( 4, (height()-pm.height())/2, pm );
  }

  if ( hasFocus() )
    p.drawRect( posx - 5, 4, width() - posx + 1 , height() - 8 );
}

QString KTagComboBox::currentTag() const
{
  return *tags->at(currentItem());
}

QString KTagComboBox::tag(int i) const
{
  if (i < 0 || i >= count())
  {
    debug("KTagComboBox::tag(), unknown tag %d", i);
    return QString::null;
  }
  return *tags->at(i);
}

int KTagComboBox::currentItem() const
{
  return current;
}

void KTagComboBox::setCurrentItem(int i)
{
  if (i < 0 || i >= count()) return;
  current = i;
  repaint();
}

void KTagComboBox::setCurrentItem(const QString &code)
{
  int i = tags->findIndex(code);
  if (code.isNull())
    i = 0;
  if (i != -1)
    setCurrentItem(i);
}

bool KTagComboBox::getMetrics( int *dist, int *buttonW, int *buttonH ) const
{
  *dist = 8;
  *buttonW = 11;
  *buttonH = 7;

  return TRUE;
}
