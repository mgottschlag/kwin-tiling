/*
 * klangcombo.h - A combobox to select a language
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


#ifndef __KLANGCOMBO_H__
#define __KLANGCOMBO_H__

#include <qwidget.h>

/*
 * This class should be just like qcombobox, but it should be possible
 * to have have a QIconSet for each entry, and each entry should have a tag.
 *
 * Support for sub menues should be added in the future.
 */
class KLanguageCombo : public QWidget
{
  Q_OBJECT

public:
  KLanguageCombo(QWidget *parent=0, const char *name=0);
  ~KLanguageCombo();

  // insertLanguage should not use locate..
  void insertLanguage(const QString& path, const QString& name, const QString& sub = QString::null);
  void changeLanguage(const QString& name, int i);

  // work space
  void insertItem(const QIconSet& icon, const QString &text, const QString &tag, int index=-1 );
  void insertSeparator(int index=-1 );
  void insertOther();
  void changeItem( const QString &text, int index );

  // count number of installed items
  int count() const;
  // clear the widget
  void clear();

  /*
   * Tag of the selected item
   */
  QString currentTag() const;
  QString tag ( int i ) const;

  /*
   * Set the current item
   */
  int currentItem() const;
  void setCurrentItem(int i);
  void setCurrentItem(const QString &code);

  // widget stuff
  QSize sizeHint() const;
signals:
  void activated( int index );
  void highlighted( int index );

private slots:
  void internalActivate( int );
  void internalHighlight( int );

protected:
  void paintEvent( QPaintEvent * );
  void mousePressEvent( QMouseEvent * );
  void keyPressEvent( QKeyEvent *e );
  void popupMenu();

private:
  // work space for the new class
  QStringList *tags;  
  QPopupMenu *popup;
  int current;
  bool getMetrics(int *dist, int *buttonW, int *buttonH) const;
};

#endif
