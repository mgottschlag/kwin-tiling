/*
  kswallow.cpp - a widget to swallow a program

  written 1997 by Matthias Hoelzer

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  */


#include <kwm.h>
#include <kwmmapp.h>

#include "kswallow.h"
#include "kswallow.moc"

KSwallowWidget::KSwallowWidget(QWidget *parent, const char *name)
  : QXEmbed(parent, name)
{
  setFocusPolicy(StrongFocus);
}

void KSwallowWidget::swallowWindow(Window w)
{
  // Define minimum size for the widged 
  // from the hints provided by window 
  XSizeHints hints;
  long flags;
  if(XGetWMNormalHints(qt_xdisplay(), w, &hints, &flags) != 0 &&
     (flags & PMinSize) != 0) {
    setMinimumSize(hints.min_width, hints.min_height);
    parentWidget()->setMinimumSize(hints.min_width, hints.min_height); 
  }

  setMinimumSize(hints.min_width, hints.min_height);

  QXEmbed::embed( w );
  show();
}
