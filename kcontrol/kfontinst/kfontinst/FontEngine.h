#ifndef __FONT_ENGINE_H__
#define __FONT_ENGINE_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontEngine
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 29/04/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <config-kfontinst.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Encodings.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <QString>
#include <QStringList>

namespace KFI
{

class CFontEngine
{
    public:

    struct TId
    {
        TId(const QString &p, int f=0) : path(p), faceNo(f) {}

        QString path;
        int     faceNo;
    };

    enum EType
    {
        // These have PS Info / support AFM stuff...
        TRUE_TYPE,
        TT_COLLECTION,
        TYPE_1,

        // These do not...
        SPEEDO,
        BITMAP,

        ANY,
        NONE
    };

    enum EWeight
    {
        WEIGHT_UNKNOWN=0,
        WEIGHT_THIN,
        WEIGHT_ULTRA_LIGHT,
        WEIGHT_EXTRA_LIGHT,
        WEIGHT_DEMI,
        WEIGHT_LIGHT,
        WEIGHT_BOOK,
        WEIGHT_MEDIUM,
        WEIGHT_REGULAR,
        WEIGHT_SEMI_BOLD,
        WEIGHT_DEMI_BOLD,
        WEIGHT_BOLD,
        WEIGHT_EXTRA_BOLD,
        WEIGHT_ULTRA_BOLD,
        WEIGHT_HEAVY,
        WEIGHT_BLACK
    };

    enum EWidth
    {
        WIDTH_UNKNOWN=0,
        WIDTH_ULTRA_CONDENSED,
        WIDTH_EXTRA_CONDENSED,
        WIDTH_CONDENSED,
        WIDTH_SEMI_CONDENSED,
        WIDTH_NORMAL,
        WIDTH_SEMI_EXPANDED,
        WIDTH_EXPANDED,
        WIDTH_EXTRA_EXPANDED,
        WIDTH_ULTRA_EXPANDED
    };

    enum ESpacing
    {
        SPACING_MONOSPACED,
        SPACING_PROPORTIONAL,
        SPACING_CHARCELL
    };

    enum EItalic
    {
        ITALIC_NONE,
        ITALIC_ITALIC,
        ITALIC_OBLIQUE
    };

    private:

    struct TFtData
    {
        struct TBitmap
        {
            TBitmap() : w(0), h(0), data(NULL) {}
            ~TBitmap() { if(data) delete data; }

            int           w,
                          h;
            unsigned char *data;
        };

        TFtData();
        ~TFtData();

        FT_Library      library;
        FT_Face         face;
        bool            open;
    };

    struct Bitmap
    {
        int           width,
                      height,
                      greys,
                      pitch;
        bool          mono;
        unsigned char *buffer;
    };

    public:

#ifdef HAVE_FONT_ENC
    CFontEngine() : itsType(NONE), itsEncodings(NULL) {}
    ~CFontEngine();
    CEncodings *   encodings();
#else
    CFontEngine() : itsType(NONE)               { }
    ~CFontEngine()                              { closeFont(); }
#endif

    static EType   getType(const char *fname);
    static QString weightStr(EWeight w);
    static QString widthStr(EWidth w);
    static QString italicStr(EItalic i)         { return ITALIC_NONE==i ? "r" : ITALIC_ITALIC==i ? "i" : "o"; }
    static QString spacingStr(ESpacing s);

    //
    // General functions - these should be used instead of specific ones below...
    //
    bool            openFont(const QString &file, int face=0);
    void            closeFont();

    //
    // These are only for non-bitmap fonts...
    const QString & getFullName()     { return itsFullName; }
    const QString & getFamilyName()   { return itsFamily; }
    const QString & getPsName()       { return itsPsName; }

    EWeight         getWeight()       { return itsWeight; }
    EWidth          getWidth()        { return itsWidth; }
    EItalic         getItalic()       { return itsItalic; }
    ESpacing        getSpacing()      { return itsSpacing; }

    const QString & getFoundry()      { return itsFoundry; }
    const QString & getAddStyle()     { return itsAddStyle; }

    EType           getType()         { return itsType; }
    int             getNumFaces()     { return itsFt.open ? itsFt.face->num_faces : 1; }

    bool            hasPsInfo()       { return itsType<=TYPE_1; }
    bool            isScaleable();
    QStringList     getEncodings();

    static EWeight  strToWeight(const char *str);
    static EWidth   strToWidth(const QString &str);

    //
    // Bitmap only
    QString &       getXlfdBmp()      { return itsFullName; }

    private:

    //
    //  TrueType & Type1 shared functionality (FreeType2)
    //
    bool            openFontFt(const QString &file);
    void            closeFaceFt();
    void            setPsNameFt();
    bool            setCharmapFt(FT_CharMap &charMap)    { return FT_Set_Charmap(itsFt.face, charMap) ? false : true; }
    unsigned int    getGlyphIndexFt(unsigned short code) { return FT_Get_Char_Index(itsFt.face, code); }
    bool            setCharmapUnicodeFt()                { return FT_Select_Charmap(itsFt.face, ft_encoding_unicode) ? false : true; }
    bool            setCharmapSymbolFt()                 { return FT_Select_Charmap(itsFt.face, ft_encoding_symbol) ? false : true; } 

#ifdef HAVE_FONT_ENC
    bool            findCharMapFt(int type, int pid, int eid);
    bool            checkEncodingFt(const QString &enc);
    bool            checkExtraEncodingFt(const QString &enc, bool found);
#else
    bool            has8BitEncodingFt(const CEncodings::T8Bit &data);
    QStringList     get8BitEncodingsFt();
#endif
    QStringList     getEncodingsFt();
    int             getNumGlyphsFt()                     { return itsFt.open ? itsFt.face->num_glyphs : 0; }

    //
    // Speedo functions...
    //
    bool            openFontSpd(const QString &file);
    QStringList     getEncodingsSpd();

    //
    // Bitmap functions...
    //
    void            createNameBmp(int pointSize, int res, const QString &enc);
    void            parseXlfdBmp();
    bool            openFontBdf(const QString &file);
    bool            openFontSnf(const QString &file);
    bool            openFontPcf(const QString &file);

    //
    // !bitmap...
    //
    void            createAddStyle();

    private:

    EWeight    itsWeight;
    EWidth     itsWidth;
    EType      itsType;
    EItalic    itsItalic;
    ESpacing   itsSpacing;
    QString    itsFullName,    // == xlfd if bitmap font!
               itsFamily,
               itsPsName,
               itsFoundry,
               itsAddStyle,
               itsPath;
    int        itsNumFaces,
               itsFaceIndex;   // Only for TTC fonts - at the moment...
    TFtData    itsFt;
#ifdef HAVE_FONT_ENC
    CEncodings *itsEncodings;
#endif
};

}

#endif
