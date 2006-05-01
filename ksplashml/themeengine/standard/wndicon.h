/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

// This file exists for the convenience of other KDE headers. It may change between
// different versions of KDE, and may disappear altogether. Do NOT include this
// directly in your program. We mean it.


#ifndef __WNDICON_H__
#define __WNDICON_H__

#include <khbox.h>
class QPixmap;

/**
 * @short Displays an icon on the screen.
 */
class WndIcon:
      public KHBox
{
  Q_OBJECT
public:

  enum Position
  {
    HBottomLeft = 0,
    HBottomRight = 1,
    HTopLeft = 2,
    HTopRight = 3,
    VBottomLeft = 10,
    VBottomRight = 11,
    VTopLeft = 12,
    VTopRight = 13
  };

  WndIcon( unsigned int, unsigned int, unsigned int, int, const QPixmap&, const QString&, Position, bool, bool );

Q_SIGNALS:
  void setStatusText( const QString& );

public Q_SLOTS:
  void show();
  void noshow();
  void slotStopJumping();
  void slotJump();

private:
  QPoint determinePosition();

protected:
  QString mStatusText;
  Position mIconPos;
  int mXineramaScreen;
  int mPosX, mPosY, mGroundX, mGroundY;
  float mVelocity, mInitialVelocity, mGravity;
  int mIconNum, mStatusHeight, mIconSize;
  bool mStatusAtTop, mStopJump, mIconJumping;
};

#endif
