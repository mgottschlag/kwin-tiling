#ifndef __FONT_ENGINE_H__
#define __FONT_ENGINE_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <fontconfig/fontconfig.h>

#if (FC_VERSION>=20402)
#define HAVE_FcFreeTypeQueryFace
#include <fontconfig/fcfreetype.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include <QtCore/QString>
#include <strigi/streambase.h>

class QByteArray;

namespace KFI
{

class CFontEngine
{
    public:

    enum EType
    {
        TYPE_OTF,
        TYPE_TTF,
        TYPE_TTC,
        TYPE_TYPE1,
        TYPE_PCF,
        TYPE_BDF,
        TYPE_AFM,

        TYPE_UNKNOWN
    };

    private:

    struct TFtData
    {
        TFtData();
        ~TFtData();

        FT_Library      library;
        FT_Face         face;
        bool            open;
    };

    public:

    static EType     getType(const char *fileName, Strigi::InputStream *in);
    static EType     getType(const char *fileName);
    static EType     getType(const char *fileName, const unsigned char *header);
    static double    decodeFixed(long v) { return (v>>16)+(((double)(v&0xFFFF))/0xFFFF); }
    static QString & fixFoundry(QString &foundry);

    CFontEngine()                     { }
    ~CFontEngine()                    { closeFont(); }

    //
    // General functions - these should be used instead of specific ones below...
    //
    bool            openFont(EType type, QByteArray &in, const char *fileName, int face=0);
    void            closeFont();

    //
    // These are only for non-bitmap fonts...
    const QString & getFamilyName()   { return itsFamily; }
    int             getWeight()       { return itsWeight; }
    int             getWidth()        { return itsWidth; }
    int             getItalic()       { return itsItalic; }
    int             getSpacing()      { return itsSpacing; }
    const QString & getFoundry()      { return itsFoundry; }
    const QString & getVersion()      { return itsVersion; }
    int             getNumFaces()     { return itsFt.open ? itsFt.face->num_faces : 1; }

    private:

    bool            openFontFt(QByteArray &in, const char *fileName, int face, bool type1);
    bool            openFontAfm(QByteArray &in);
#ifndef HAVE_FcFreeTypeQueryFace
    void            parseXlfdBmp(const QString &lfd);
    bool            openFontBdf(QByteArray &in);
    bool            openFontPcf(QByteArray &in);
#endif

    private:

    int        itsWeight,
               itsWidth,
               itsItalic,
               itsSpacing;
    QString    itsFamily,
               itsFoundry,
               itsVersion;
    TFtData    itsFt;
};

}

#endif
