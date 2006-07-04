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

#include <config.h>

#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kwin.h>

#include <qdesktopwidget.h>

#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QTimer>
#include <QWidget>
//Added by qt3to4:
#include <QBitmap>

#include "wndicon.h"
#include "wndicon.moc"

WndIcon::WndIcon(
  unsigned int icon_num,
  unsigned int icon_std_width,
  unsigned int status_height,
  int xineramaScreen,
  const QPixmap& pix,
  const QString& text,
  Position icon_position,
  bool statusAtTop,
  bool iconsJumping )
    :KHBox( 0 ),
     mStatusText(text), mIconPos(icon_position), mXineramaScreen( xineramaScreen ), mPosX(0), mPosY(0), mGroundX(0), mGroundY(0),
     mVelocity(8.0), mInitialVelocity(8.0), mGravity(0.8),
     mIconNum(icon_num), mStatusHeight(status_height), mIconSize(icon_std_width), mStatusAtTop(statusAtTop),
     mStopJump(false), mIconJumping(iconsJumping)
{
  setWindowFlags (Qt::WStyle_Customize|Qt::WX11BypassWM );
  QLabel *w = new QLabel( this );
  w->setFixedSize( pix.width(), pix.height() );
  w->setPixmap( pix );
  if(!pix.mask().isNull())
  {
    setMask(pix.mask());
    w->setMask(pix.mask());
  }

  resize( pix.width(), pix.height() );

  // Set initial position of icon, and ground.
  QPoint p = determinePosition();
  mGroundX = mPosX = p.x();
  mGroundY = mPosY = p.y();
  move( p + kapp->desktop()->screenGeometry( mXineramaScreen ).topLeft() );

  if( mIconJumping )
  {
    QTimer *t = new QTimer( this );
    connect(t, SIGNAL(timeout()), SLOT(slotJump()));
    t->setSingleShot( false );
    t->start( 50 );
  }
}

void WndIcon::show()
{
  emit setStatusText( mStatusText );
  KHBox::show();
}

// Emit our EXTRA signal without becoming visible.
void WndIcon::noshow()
{
  emit setStatusText( mStatusText );
}

/*
 * WndIcon::determinePosition(): Based on the information we've
 * stored within us, determine where we should be placed on the
 * screen. This is placed in its own function due to the massive
 * complexity required by the variables involved.
 */
QPoint WndIcon::determinePosition()
{
  int DW, DH, SBH, wid, X, Y, x, y, nSlot, topshift, bottomshift;

  bottomshift = topshift = 0;
  const QRect srect = kapp->desktop()->screenGeometry( mXineramaScreen );
  // KGlobalSettings::splashScreenDesktopGeometry(); cannot be used here.
  DW = srect.width();
  DH = srect.height();
  SBH = mStatusHeight;
  wid = mIconSize;
  x = mIconNum;
  y = 1;

  if(mStatusAtTop)
    topshift = mStatusHeight;
  else
    bottomshift = mStatusHeight;

  // Different starting positions require different positioning
  // rules. The horizontal rules and the vertical rules can be
  // grouped together, as they are innately similar.
  switch( mIconPos )
  {
    // HORIZONTAL: Top Left -> Top Right
  case( WndIcon::HTopLeft ):
    nSlot = ( DW / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (3) + ((x - 1) * wid);
    Y = topshift + 3 + ( (y-1) * wid );
    break;

    // HORIZONTAL: Bottom Left -> Bottom Right
  case( WndIcon::HBottomLeft ):
    nSlot = ( DW / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (3) + ((x - 1) * wid);
    Y = (DH-3) - (y * wid) - bottomshift;
    break;

    // HORIZONTAL: Top Right -> Top Left
  case( WndIcon::HTopRight ):
    nSlot = ( DW / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (DW - 3) - (( x ) * wid );
    Y = topshift + (3) + ( (y-1) * wid );
    break;

    // HORIZONTAL: Bottom Right -> Bottom Left
  case( WndIcon::HBottomRight ):
    nSlot = ( DW / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (DW - 3) - (( x ) * wid );
    Y = (DH-3) - (y * wid) - bottomshift;
    break;

    // VERTICAL: Top Left -> Bottom Left
  case( WndIcon::VTopLeft ):
    nSlot = ( DH / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (3) + (( y - 1 ) * wid );
    Y = topshift + (3) + ((x-1) * wid);
    break;

    // VERTICAL: Top Right -> Bottom Right
  case( WndIcon::VTopRight ):
    nSlot = ( DH / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (DW - 3) - (( y ) * wid );
    Y = topshift + (3) + ((x-1) * wid);
    break;

    // VERTICAL: Bottom Left -> Top Left
  case( WndIcon::VBottomLeft ):
    nSlot = ( DH / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (3) + (( y - 1 ) * wid );
    Y = (DH-3) - (x * wid) - bottomshift;
    break;

    // VERTICAL: Bottom Right -> Top Right
  case( WndIcon::VBottomRight ):
    nSlot = ( DH / wid );
    while( x > nSlot )
    {
      x = qMax(0,x-nSlot);
      y++;
    }
    X = (DW - 3) - (( y ) * wid );
    Y = (DH-3) - (x * wid) - bottomshift;
    break;

    // Unknown Condition. Reconfigure variable and
    // retry. Recursively, of course! ;)
  default:
    mIconPos = HBottomLeft;
    return(determinePosition());
    break;
  }
  return( QPoint( X, Y ) );
}

// Make the icons jump in a macosx-like way (Thanks, Jone!:)
void WndIcon::slotJump()
{
  // Based on our position, bounce around.
  switch( mIconPos )
  {
    // LEFT
  case( WndIcon::VTopLeft ):
  case( WndIcon::VBottomLeft ):
    mVelocity -= mGravity;
    if( mStopJump )
    {
      if( mPosX <= mGroundX )
      {
        mPosX = mGroundX;
        mVelocity = mGravity = 0.0;
      }
    }
    else
      if( mPosX <= mGroundX )
        mVelocity = mInitialVelocity;
    mPosX = (int)((float)mPosX + mVelocity);
    break;

    // RIGHT
  case( WndIcon::VTopRight ):
  case( WndIcon::VBottomRight ):
    mVelocity -= mGravity;
    if( mStopJump )
    {
      if( mPosX >= mGroundX )
      {
        mPosX = mGroundX;
        mVelocity = mGravity = 0.0;
      }
    }
    else
      if( mPosX >= mGroundX )
        mVelocity = mInitialVelocity;
    mPosX = (int)((float)mPosX - mVelocity);
    break;

    // TOP
  case( WndIcon::HTopLeft ):
  case( WndIcon::HTopRight ):
    mVelocity -= mGravity;
    if( mStopJump )
    {
      if( mPosY <= mGroundY )
      {
        mPosY = mGroundY;
        mVelocity = mGravity = 0.0;
      }
    }
    else
      if( mPosY <= mGroundY )
        mVelocity = mInitialVelocity;
    mPosY = (int)((float)mPosY + mVelocity );
    break;

    // BOTTOM
  case( WndIcon::HBottomLeft ):
  case( WndIcon::HBottomRight ): default:
    mVelocity -= mGravity;
    if( mStopJump )
    {
      if( mPosY >= mGroundY )
      {
        mPosY = mGroundY;
        mVelocity = mGravity = 0.0;
      }
    }
    else
      if( mPosY >= mGroundY )
        mVelocity = mInitialVelocity;
    mPosY = (int)((float)mPosY - mVelocity );
    break;
  }
  move( QPoint( mPosX, mPosY ) + kapp->desktop()->screenGeometry( mXineramaScreen ).topLeft() );
}

void WndIcon::slotStopJumping()
{
  mStopJump = true;
}
