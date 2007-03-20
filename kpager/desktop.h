/**************************************************************************

    desktop.h  - KPager's desktop
    Copyright (C) 2000  Antonio Larrosa Jimenez <larrosa@kde.org>
			Matthias Ettrich
			Matthias Elter

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Send comments and bug fixes to larrosa@kde.org

***************************************************************************/
#ifndef __DESKTOP_H
#define __DESKTOP_H

#include <QWidget>
#include <q3intdict.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <kwin.h>

class KSharedPixmap;
class KMenu;

class QPainter;
class QPoint;

class Desktop : public QWidget
{
    Q_OBJECT

public:
  Desktop( int desk, const QString &desktopName, QWidget *parent=0 );
  ~Desktop();

  int id() const { return m_desk; }
  bool isCurrent() const;

//  int widthForHeight(int height) const;
//  int heightForWidth(int width) const;

  static const bool c_defShowName;
  static const bool c_defShowNumber;
  static const bool c_defShowBackground;
  static const bool c_defShowWindows;
  static const bool c_defWindowDragging;
  enum WindowDrawMode { Plain=0, Icon=1, Pixmap=2 };
  enum WindowTransparentMode { NoWindows=0, MaximizedWindows=1, AllWindows=2};
  static const WindowDrawMode c_defWindowDrawMode;
  static const WindowTransparentMode c_defWindowTransparentMode;

  virtual int deskX() const { return 0; }
  virtual int deskY() const { return 0; }
  virtual int deskWidth() const { return width(); }
  virtual int deskHeight() const { return height(); }

  void startDrag(const QPoint &point);
  void dragEnterEvent(QDragEnterEvent *ev);
  void dragMoveEvent(QDragMoveEvent *);
  void dropEvent(QDropEvent *ev);
  void convertRectS2P(QRect &r);
  void convertCoordP2S(int &x, int &y);

	static void removeCachedPixmap(int nWin) { m_windowPixmaps.remove(nWin); }

  QSize sizeHint() const;

  /**
   * active is a bool that specifies if the frame is the active
   * one or not (so that it's painted highlighted or not)
   */
  void paintFrame(bool active);

  bool m_grabWindows;
public Q_SLOTS:
  void backgroundLoaded(bool b);

  void loadBgPixmap();

protected:
  void mousePressEvent( QMouseEvent *ev );
  void mouseMoveEvent( QMouseEvent *ev );
  void mouseReleaseEvent( QMouseEvent *ev );

  void paintEvent( QPaintEvent *ev );

  KWin::WindowInfo *windowAtPosition (const QPoint &p, QPoint *internalpos);

  bool shouldPaintWindow( KWin::WindowInfo *info );

  int m_desk;
  QString m_name;
  KSharedPixmap *m_bgPixmap;
  bool m_bgDirty;
  QPixmap *m_bgSmallPixmap;
  static QPixmap *m_bgCommonSmallPixmap;
  static bool m_isCommon;
  static Q3IntDict<QPixmap> m_windowPixmaps;
  static QMap<int,bool> m_windowPixmapsDirty;
  WindowTransparentMode m_transparentMode;

  QPixmap *paintNewWindow(const KWin::WindowInfo *info);

  void paintWindow(QPainter &p, const KWin::WindowInfo *info,
			bool onDesktop=true);
  void paintWindowPlain(QPainter &p, const KWin::WindowInfo *info,
			bool onDesktop=true);
  void paintWindowIcon(QPainter &p, const KWin::WindowInfo *info,
			bool onDesktop=true);
  void paintWindowPixmap(QPainter &p, const KWin::WindowInfo *info,
			bool onDesktop=true);

private:
  class KPager* pager() const;
    QPoint pressPos;

};

#endif
