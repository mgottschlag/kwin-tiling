/* kasbar.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#include <kstandarddirs.h>
#include <kpixmapeffect.h>

#include "kasbar.h"

#include "kasresources.h"
#include "kasresources.moc"
//Added by qt3to4:
#include <QPixmap>

//
// Bitmap data used for the window state indicators
//
static unsigned char min_bits[] = {
    0x00, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00};
static unsigned char max_bits[] = {
    0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff};
static unsigned char shade_bits[] = {
    0x06, 0x1e, 0x7e, 0xfe, 0xfe, 0x7e, 0x1e, 0x06};

static unsigned char attention_bits[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/* XPM */
static const char *tiny_floppy[]={
"10 10 4 1",
". c None",
"# c #000000",
"b c #a0a0a0",
"a c #ffffff",
".########.",
".#aaaaaa#.",
".#aaaaaa#.",
".#aaaaaa#.",
".########.",
".########.",
".##bbbb##.",
".##bbbb##.",
".##bbbb##.",
".........."};

static const char *micro_max[]={
"6 6 2 1",
". c None",
"# c #000000",
"######",
"######",
"##..##",
"##..##",
"######",
"######",
};

static const char *micro_min[]={
"6 6 2 1",
". c None",
"# c #000000",
"......",
"######",
"######",
".####.",
"..##..",
"......"
};

static const char *micro_shade[]={
"6 6 2 1",
". c None",
"# c #000000",
".##...",
".###..",
".####.",
".####.",
".###..",
".##..."
};

KasResources::KasResources( KasBar *parent, const char *name )
    : QObject( parent, name ? name : "kasbar_resources" ),
      kasbar( parent ),
      labelPenColor_( Qt::white ), labelBgColor_( Qt::black ),
      activePenColor_( Qt::black ), activeBgColor_( Qt::white ),
      inactivePenColor_( Qt::black ), inactiveBgColor_( Qt::white ),
      progressColor_( Qt::green ), attentionColor_( Qt::red ),
      startupFrames_()
{
}

KasResources::~KasResources()
{
}

QBitmap KasResources::minIcon()
{
   if ( minPix.isNull() ) {
      minPix = QBitmap(8, 8, min_bits, true);
      minPix.setMask(minPix);
   }

   return minPix;
}

QBitmap KasResources::maxIcon()
{
   if ( maxPix.isNull() ) {
      maxPix = QBitmap(8, 8, max_bits, true);
      maxPix.setMask(maxPix);
   }

   return maxPix;
}

QBitmap KasResources::shadeIcon()
{
   if ( shadePix.isNull() ) {
      shadePix = QBitmap(8, 8, shade_bits, true);
      shadePix.setMask(shadePix);
   }

   return shadePix;
}

QBitmap KasResources::attentionIcon()
{
   if ( attentionPix.isNull() ) {
      attentionPix = QBitmap( 8, 8, attention_bits, true );
      attentionPix.setMask( attentionPix );
   }

   return attentionPix;
}

QPixmap KasResources::modifiedIcon()
{
   if ( modifiedPix.isNull() )
      modifiedPix = QPixmap( tiny_floppy );

   return modifiedPix;
}

QPixmap KasResources::microShadeIcon()
{
  if ( microShadePix.isNull() )
    microShadePix = QPixmap( micro_shade );

  return microShadePix;
}

QPixmap KasResources::microMaxIcon()
{
  if ( microMaxPix.isNull() )
    microMaxPix = QPixmap( micro_max );

  return microMaxPix;
}

QPixmap KasResources::microMinIcon()
{
  if ( microMinPix.isNull() )
    microMinPix = QPixmap( micro_min );

  return microMinPix;
}

static const int MAX_ANIMATION_FRAME=10;

Q3ValueVector<QPixmap> KasResources::startupAnimation()
{
    if ( startupFrames_.isEmpty() ) {
	for ( int i = 1; i <= MAX_ANIMATION_FRAME; i++ ) {
	    QPixmap p( locate("data", "kicker/pics/disk" + QString::number(i) + ".png") );
	    if ( !p.isNull() )
		startupFrames_.append( p );
	}
    }

    return startupFrames_;
}

void KasResources::setLabelPenColor( const QColor &color )
{
    if ( labelPenColor_ == color )
	return;

    labelPenColor_ = color;
    emit changed();
}

void KasResources::setLabelBgColor( const QColor &color )
{
    if ( labelBgColor_ == color )
	return;

    labelBgColor_ = color;
    emit changed();
}

void KasResources::setInactivePenColor( const QColor &color )
{
    if ( inactivePenColor_ == color )
	return;

    inactivePenColor_ = color;
    emit changed();
}

void KasResources::setInactiveBgColor( const QColor &color )
{
    if ( inactiveBgColor_ == color )
	return;

    inactiveBgColor_ = color;
    emit changed();
}

void KasResources::setActivePenColor( const QColor &color )
{
    if ( activePenColor_ == color )
	return;

    activePenColor_ = color;
    emit changed();
}

void KasResources::setActiveBgColor( const QColor &color )
{
    if ( activeBgColor_ == color )
	return;

    activeBgColor_ = color;
    emit changed();
}

void KasResources::setProgressColor( const QColor &color )
{
    if ( progressColor_ == color )
	return;

    progressColor_ = color;
    emit changed();
}

void KasResources::setAttentionColor( const QColor &color )
{
    if ( attentionColor_ == color )
	return;

    attentionColor_ = color;
    emit changed();
}

void KasResources::itemSizeChanged()
{
    actBg = KPixmap();
    inactBg = KPixmap();
}

KPixmap KasResources::activeBg()
{
   if ( actBg.isNull() ) {
      actBg.resize( kasbar->itemExtent(), kasbar->itemExtent() );
      KPixmapEffect::gradient( actBg,
			       kasbar->colorGroup().light(), kasbar->colorGroup().mid(),
			       KPixmapEffect::DiagonalGradient );
   }

   return actBg;
}

KPixmap KasResources::inactiveBg()
{
   if ( inactBg.isNull() ) {
      inactBg.resize( kasbar->itemExtent(), kasbar->itemExtent() );
      KPixmapEffect::gradient( inactBg,
			       kasbar->colorGroup().mid(), kasbar->colorGroup().dark(),
			       KPixmapEffect::DiagonalGradient );
   }

   return inactBg;
}

