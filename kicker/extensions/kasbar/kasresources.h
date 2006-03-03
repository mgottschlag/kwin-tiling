// -*- c++ -*-

/* kasbar.h
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

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/

#ifndef KASRESOURCES_H
#define KASRESOURCES_H

#include <qbitmap.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qpen.h>
//Added by qt3to4:
#include <QPixmap>
#include <kpixmap.h>

#include <qobject.h>


class KasBar;

/**
 * Central class that holds the graphical resources for the bar.
 *
 * @author Richard Moore, rich@kde.org
 */
class KasResources : public QObject
{
    Q_OBJECT

public:
    KasResources( KasBar *parent, const char *name=0 );
    virtual ~KasResources();

    QColor labelPenColor() const    { return labelPenColor_; }
    QColor labelBgColor() const     { return labelBgColor_; }
    QColor inactivePenColor() const { return inactivePenColor_; }
    QColor inactiveBgColor() const  { return inactiveBgColor_; }
    QColor activePenColor() const   { return activePenColor_; }
    QColor activeBgColor() const    { return activeBgColor_; }

    QColor progressColor() const    { return progressColor_; }      
    QColor attentionColor() const    { return attentionColor_; }      

    /** Accessor for the min icon (singleton). */
    QBitmap minIcon();

    /** Accessor for the max icon (singleton). */
    QBitmap maxIcon();

    /** Accessor for the shade icon (singleton). */
    QBitmap shadeIcon();

    /** Accessor for the attention icon (singleton). */
    QBitmap attentionIcon();

    /** Accessor for the modified icon (singleton). */
    QPixmap modifiedIcon();

    /** Accessor for the micro min icon (singleton). */
    QPixmap microMinIcon();

    /** Accessor for the micro max icon (singleton). */
    QPixmap microMaxIcon();

    /** Accessor for the micro shade icon (singleton). */
    QPixmap microShadeIcon();

    /** Accessor used by items to get the active bg fill. */
    KPixmap activeBg();
    
    /** Accessor used by items to get the inactive bg fill. */
    KPixmap inactiveBg();

    Q3ValueVector<QPixmap> startupAnimation();

public Q_SLOTS:
    void setLabelPenColor( const QColor &color );
    void setLabelBgColor( const QColor &color );
    void setInactivePenColor( const QColor &color );
    void setInactiveBgColor( const QColor &color );
    void setActivePenColor( const QColor &color );
    void setActiveBgColor( const QColor &color );

    void setProgressColor( const QColor &color );
    void setAttentionColor( const QColor &color );

    void itemSizeChanged();

Q_SIGNALS:
    void changed();

private:
    KasBar *kasbar;

    QBitmap minPix;
    QBitmap maxPix;
    QBitmap shadePix;
    QBitmap attentionPix;
    QPixmap modifiedPix;
    QPixmap microShadePix;
    QPixmap microMaxPix;
    QPixmap microMinPix;

    QColor labelPenColor_;
    QColor labelBgColor_;
    QColor activePenColor_;
    QColor activeBgColor_;
    QColor inactivePenColor_;
    QColor inactiveBgColor_;

    QColor progressColor_;
    QColor attentionColor_;

    KPixmap actBg;
    KPixmap inactBg;

    Q3ValueVector<QPixmap> startupFrames_;
};

#endif // KASRESOURCES_H

