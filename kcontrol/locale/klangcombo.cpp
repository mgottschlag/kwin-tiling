/*
 * klangcombo.cpp - A combobox to select a language
 *
 * Copyright (c) 1999 Hans Petter Bieker <bieker@kde.org>
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

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qpopupmenu.h>

#include <kiconloader.h>
#include <kapp.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klangcombo.h"
#include "klangcombo.moc"

KLanguageCombo::~KLanguageCombo ()
{
  delete popup;
  delete tags;
}

KLanguageCombo::KLanguageCombo (QWidget * parent, const char *name)
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

void KLanguageCombo::popupMenu()
{
   popup->popup( mapToGlobal( QPoint(0,0) ), current );
}

void KLanguageCombo::keyPressEvent( QKeyEvent *e )
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

void KLanguageCombo::mousePressEvent( QMouseEvent * /*e*/ )
{
  popupMenu();
}

QSize KLanguageCombo::sizeHint() const
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

void KLanguageCombo::internalActivate( int index )
{
  if (current == index) return;
  current = index;
  repaint();
  emit activated( index );
}

void KLanguageCombo::internalHighlight( int index )
{
  emit highlighted( index );
}

void KLanguageCombo::clear()
{
  popup->clear();
  tags->clear();
}

int KLanguageCombo::count() const
{
  return popup->count();
}

void KLanguageCombo::insertItem(const QIconSet& icon, const QString &text, const QString &tag, int index)
{
  if (index < 0 || index >= count())
    index = count();
  popup->insertItem(icon, text, index, index);
  tags->insert(tags->at(index), tag);
}

void KLanguageCombo::changeItem( const QString &text, int index )
{
  popup->changeItem( text, index);
}

void KLanguageCombo::paintEvent( QPaintEvent * )
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

void KLanguageCombo::insertSeparator(int index)
{
  if (index < 0 || index >= count()) index = count();
  popup->insertSeparator(index);
  tags->insert(tags->at(index), QString::null);
}

void KLanguageCombo::insertLanguage(const QString& path, const QString& name, const QString& sub)
{
  if (path.isNull())
    insertSeparator();
  else
  {
    QString output = name + " (" + path + ")";
    QPixmap flag(locate("locale", sub + path + "/flag.png"));
    insertItem(QIconSet(flag), output, path);
  }
}

// does not work
void KLanguageCombo::changeLanguage(const QString& name, int i)
{
  if (i < 0 || i >= count()) return;
  QString output = name + " (" + *tags->at(i) + ")";
  changeItem(output, i);
}

QString KLanguageCombo::currentTag() const
{
  return *tags->at(currentItem());
}

QString KLanguageCombo::tag(int i) const
{
  if (i < 0 || i >= count()) return 0;
  return *tags->at(i);
}

int KLanguageCombo::currentItem() const
{
  return current;
}

void KLanguageCombo::setCurrentItem(int i)
{
  if (i < 0 || i >= count()) return;
  current = i;
  repaint();
}

void KLanguageCombo::setCurrentItem(const QString &code)
{
  int i = tags->findIndex(code);
  if (code.isNull())
    i = 0;
  if (i != -1)
    setCurrentItem(i);
}

bool KLanguageCombo::getMetrics( int *dist, int *buttonW, int *buttonH ) const
{
  *dist = 8;
  *buttonW = 11;
  *buttonH = 7;

  return TRUE;
}
