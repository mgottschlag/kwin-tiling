/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */
#ifndef __BGDefaults_h_Included__
#define __BGDefaults_h_Included__


// Globals
#define _maxDesktops 8

#define _defCommon false
#define _defDock true
#define _defExport false
#define _defLimitCache true
#define _defCacheSize 2048

// Per desktop defaults
// Before you change this get in touch with me (torsten@kde.org)
// Thanks!!
#define _defColorA  QColor("#249FD0")
#define _defColorB  QColor("#19A2A0")
#define _defBackgroundMode VerticalGradient
#define _defWallpaperMode NoWallpaper
#define _defMultiMode NoMulti
#define _defBlendMode NoBlending
#define _defBlendBalance 100
#define _defReverseBlending false

#endif // __BGDefaults_h_Included__
