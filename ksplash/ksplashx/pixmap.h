/****************************************************************************
**
** This file is based on sources of the Qt GUI Toolkit, used under the terms
** of the GNU General Public License version 2 (see the original copyright
** notice below).
** All further contributions to this file are (and are required to be)
** licensed under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** The original Qt license header follows:
**
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef _PIXMAP_H
#define _PIXMAP_H

#include "qnamespace.h"

#include "x11_defs.h"

class QImage;

enum Optimization { DefaultOptim, NoOptim, MemoryOptim=NoOptim,
			NormalOptim, BestOptim };

struct PixmapData
    {
    PixmapData() : hd( None ), optim( NoOptim ), ximage( NULL ) {}
    Pixmap hd;
    int w;
    int h;
    int d;
    Optimization optim;
    XImage* ximage;
    };

class PP : public Qt // inherit from Qt to reduce needed code changes in the function
    {
    public:
        static PixmapData convertFromImage( const QImage& img, int conversion_flags );
    };

inline
PixmapData imageToPixmap( const QImage& im )
    {
    return PP::convertFromImage( im, 0 );
    }

QImage splash_read_png_image(FILE* f);
QImage splash_read_jpeg_image(FILE* f);

#endif
