/*
 * klangcombo.cpp - A combobox to select a language
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
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
#include <qpixmap.h>
#include <kiconloader.h>
#include <kapp.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klangcombo.h"
#include "klangcombo.moc"

KLanguageCombo::~KLanguageCombo ()
{
}

KLanguageCombo::KLanguageCombo (QWidget * parent, const char *name)
  : QComboBox(parent, name)
{
}

void KLanguageCombo::insertLanguage(const QString& path, const QString& name, const QString& sub)
{
  QPainter p;

  QString output = name + " (" + path + ")";

  int w = fontMetrics().width(output) + 24;
  QPixmap pm(w, 16);

  QPixmap flag(locate("locale", sub + path + "/flag.png"));
  pm.fill(colorGroup().background());
  p.begin(&pm);

  p.drawText(24, 1 , w-24, 16, AlignLeft | AlignTop, output);
  if (!flag.isNull())
    p.drawPixmap(1,1,flag);
  p.end();
  insertItem(pm);
  tags.append(path);
}

QString KLanguageCombo::currentTag() const
{
  return *tags.at(currentItem());
}

void KLanguageCombo::setItem(const QString &code)
{
  int i = tags.findIndex(code);
  if (code.isNull())
    i = 0;
  if (i != -1)
    setCurrentItem(i);
}
