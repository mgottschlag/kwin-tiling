///////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontEngine
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

//
// This class contains code inspired/copied/nicked from mkfontscale. Specifically
// the getName(), lookupName(), getFoundry(), and the encoding routines within
// HAVE_FONT_ENC #defines...
//
// mkfontscale's (C) notice is:
/*
  Copyright (c) 2002 by Juliusz Chroboczek

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
    
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/  

#include "FontEngine.h"
#include "Global.h"
#include "KfiConfig.h"
#include "Misc.h"
#include "CompressedFile.h"
#include <kurl.h>
#include <kconfig.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <qregexp.h>
#include <qfile.h>
#include <ft2build.h>
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPE1_TABLES_H

#ifdef HAVE_FONT_ENC
extern "C"
{
#include <X11/fonts/fontenc.h>
}
#endif

#ifdef HAVE_FT_CACHE
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <kglobalsettings.h>
#include <klocale.h>
#endif

static const char *constNoPsName             = "NO_PS_NAME";
static const char *constBmpRoman             = "";
static const char *constBmpItalic            = " Italic";
static const char *constBmpOblique           = " Oblique";
static const char *constOblique              = "Oblique";
static const char *constSlanted              = "Slanted";
static const char *const constDefaultFoundry = "Misc";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    GENERIC
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_FT_CACHE
extern "C"
{
    static FT_Error face_requester(FTC_FaceID faceId, FT_Library lib, FT_Pointer ptr, FT_Face *face)
    {
        CFontEngine::TId *id=(CFontEngine::TId *)faceId;
        FT_Error            err=FT_New_Face(lib, QFile::encodeName(id->path), id->faceNo, face);

        ptr=ptr;

        if(err)
        {
            const QStringList          &list=CGlobal::cfg().getRealTopDirs(id->path);
            QStringList::ConstIterator it;

            for(it=list.begin(); it!=list.end(); it++)
                if((err=FT_New_Face(lib, QFile::encodeName(*it+CMisc::getSub(id->path)), id->faceNo, face)))
                    break;
        }
        return err;
    }
}
#endif

bool CFontEngine::openFont(const QString &file, unsigned short mask, bool force, int face)
{
    bool ok=false;

    KFI_DBUG << "openFont(" << file << ')' << endl;

    if(file==itsPath && face==itsFaceIndex)
    {
        KFI_DBUG << "openFont, same as previous - therefore not reopening" << endl;
        ok=NONE!=itsType;
    }
    else
    {
        closeFont();

        itsType=getType(QFile::encodeName(file));
        itsWeight=WEIGHT_MEDIUM;
        itsWidth=WIDTH_NORMAL;
        itsSpacing=SPACING_PROPORTIONAL;
        itsItalic=ITALIC_NONE;
        itsAddStyle=QString::null;
        itsPixelSize=0;
        itsPath=file;
        itsFaceIndex=face;

        switch(itsType)
        {
            case TRUE_TYPE:
            case TT_COLLECTION:
            case OPEN_TYPE:
                ok=openFontTT(file, mask);
                break;
            case TYPE_1:
                ok=openFontT1(file, mask);
                break;
            case SPEEDO:
                ok=openFontSpd(file, mask);
                break;
            case BDF:
                ok=openFontBdf(file);
                break;
            case PCF:
                ok=openFontPcf(file);
                break;
            case SNF:
                ok=openFontSnf(file);
                break;
            case TYPE_1_AFM:
                ok=openFontAfm(file);
                break;
            default:
                if(force)
                    if((ok=openFontT1(file, mask)))
                        itsType=TYPE_1;
                    else if((ok=openFontTT(file, mask)))
                        itsType=itsFt.face->num_faces>1 ? TT_COLLECTION : TRUE_TYPE;
                    else if((ok=openFontAfm(file)))
                        itsType=TYPE_1_AFM;
                    else if((ok=openFontPcf(file)))
                        itsType=PCF;
                    else if((ok=openFontSpd(file)))
                        itsType=SPEEDO;
                    else if((ok=openFontBdf(file)))
                        itsType=BDF;
                    else if((ok=openFontSnf(file)))
                        itsType=SNF;
        }
    }

    KFI_DBUG << "openFont, status:" << ok << endl;
    return ok;
}

bool CFontEngine::openFont(const KURL &url, unsigned short mask, bool force, int face)
{
    KFI_DBUG << "openFont(" << url.protocol() << ":" << url.path() << ')' << endl;

    if(KIO_FONTS_PROTOCOL!=url.protocol())
        return openFont(url.path(), mask, force, face);
    else
    {
        const QStringList          &list=CGlobal::cfg().getRealTopDirs(url.path());
        QStringList::ConstIterator it;


        for(it=list.begin(); it!=list.end(); it++)
        {
            QString fname(*it+CMisc::getSub(url.path()));

            if(CMisc::fExists(fname) && openFont(fname, mask, force, face))
            {
                itsPath=fname;
                return true;
            }
        }
    }
    return false; 
}

void CFontEngine::closeFont()
{
    closeFaceFt();
    itsPath=QString::null;
    itsFaceIndex=-1;
    itsType=NONE;
}

bool CFontEngine::isA(const char *fname, const char *ext, bool z)
{
    int  len=strlen(fname);
    bool fnt=false;

    if(z)
    {
        if(len>7)                 // Check for .ext.gz
            fnt=(fname[len-7]=='.' && tolower(fname[len-6])==ext[0] && tolower(fname[len-5])==ext[1] &&
                 tolower(fname[len-4])==ext[2] && fname[len-3]=='.' && tolower(fname[len-2])=='g' &&
                 tolower(fname[len-1])=='z');

        if(!fnt && len>6)         // Check for .ext.Z
            fnt=(fname[len-6]=='.' && tolower(fname[len-5])==ext[0] && tolower(fname[len-4])==ext[1] &&
                 tolower(fname[len-3])==ext[2] && fname[len-2]=='.' && toupper(fname[len-1])=='Z');
    }

    if(!fnt && len>4)  // Check for .ext
        fnt=(fname[len-4]=='.' && tolower(fname[len-3])==ext[0] && tolower(fname[len-2])==ext[1] &&
             tolower(fname[len-1])==ext[2]);

    return fnt;
}

bool CFontEngine::isAFont(const char *fname)
{
    EType type=getType(fname);

    return isAFont(type);
}

CFontEngine::EType CFontEngine::getType(const char *fname)
{
    if(isATtf(fname))
        return TRUE_TYPE;
    if(isATtc(fname))
        return TT_COLLECTION;
    if(isAOtf(fname))
        return OPEN_TYPE;
    if(isAType1(fname))
        return TYPE_1;
    if(isAAfm(fname))
        return TYPE_1_AFM;
    if(isAPcf(fname))
        return PCF;
    if(isASpeedo(fname))
        return SPEEDO;
    if(isABdf(fname))
        return BDF;
    if(isASnf(fname))
        return SNF;
    else
        return NONE;
}

QString CFontEngine::weightStr(enum EWeight w)
{
    switch(w)
    {
        case WEIGHT_THIN:
            return "Thin";
        case WEIGHT_ULTRA_LIGHT:
            return "UltraLight";
        case WEIGHT_EXTRA_LIGHT:
            return "ExtraLight";
        case WEIGHT_DEMI:
            return "Demi";
        case WEIGHT_LIGHT:
            return "Light";
        case WEIGHT_BOOK:
            return "Book";
        case WEIGHT_MEDIUM:
            return "Medium";
        case WEIGHT_REGULAR:
            return "Regular";
        case WEIGHT_SEMI_BOLD:
            return "SemiBold";
        case WEIGHT_DEMI_BOLD:
            return "DemiBold";
        case WEIGHT_BOLD:
            return "Bold";
        case WEIGHT_EXTRA_BOLD:
            return "ExtraBold";
        case WEIGHT_ULTRA_BOLD:
            return "UltraBold";
        case WEIGHT_HEAVY:
            return "Heavy";
        case WEIGHT_BLACK:
            return "Black";
        case WEIGHT_UNKNOWN:
        default:
            return "Medium";
    }
}

QString CFontEngine::widthStr(enum EWidth w)
{
    switch(w)
    {
        case WIDTH_ULTRA_CONDENSED:
            return "UltraCondensed";
        case WIDTH_EXTRA_CONDENSED:
            return "ExtraCondensed";
        case WIDTH_CONDENSED:
            return "Condensed";
        case WIDTH_SEMI_CONDENSED:
            return "SemiCondensed";
        case WIDTH_SEMI_EXPANDED:
            return "SemiExpanded";
        case WIDTH_EXPANDED:
            return "Expanded";
        case WIDTH_EXTRA_EXPANDED:
            return "ExtraExpanded";
        case WIDTH_ULTRA_EXPANDED:
            return "UltraExpanded";
        case WIDTH_NORMAL:
        case WIDTH_UNKNOWN:
        default:
            return "Normal";
    }
}

QString CFontEngine::spacingStr(enum ESpacing s)
{
    switch(s)
    {
        case SPACING_MONOSPACED:
            return "m";
        case SPACING_CHARCELL:
            return "c";
        default:
        case SPACING_PROPORTIONAL:
            return "p";
    }
}

bool CFontEngine::isScaleable()
{
    switch(itsType)
    {
        case TRUE_TYPE:
        case TT_COLLECTION:
        case OPEN_TYPE:
        case TYPE_1:
            return itsFt.open && FT_IS_SCALABLE(itsFt.face);
        case SPEEDO:
            return true;
        case PCF:
        case BDF:
        case SNF:
        default:
            return false;
    }
}

QStringList CFontEngine::getEncodings()
{
    switch(itsType)
    {
        case TRUE_TYPE:
        case TT_COLLECTION:
        case OPEN_TYPE:
        case TYPE_1:
            return getEncodingsFt();
        case SPEEDO:
            return getEncodingsSpd();
        case PCF:
        case BDF:
        case SNF:
        default:
        {
            QStringList empty;

            return empty;
        }
    }
}

CFontEngine::EWeight CFontEngine::strToWeight(const char *str)
{
    if(NULL==str)
        return WEIGHT_MEDIUM; // WEIGHT_UNKNOWN;
    else if(CMisc::stricmp(str, "Bold")==0)
        return WEIGHT_BOLD;
    else if(CMisc::stricmp(str, "Black")==0)
        return WEIGHT_BLACK;
    else if(CMisc::stricmp(str, "ExtraBold")==0)
        return WEIGHT_EXTRA_BOLD;
    else if(CMisc::stricmp(str, "UltraBold")==0)
        return WEIGHT_ULTRA_BOLD;
    else if(CMisc::stricmp(str, "ExtraLight")==0)
        return WEIGHT_EXTRA_LIGHT;
    else if(CMisc::stricmp(str, "UltraLight")==0)
        return WEIGHT_ULTRA_LIGHT;
    else if(CMisc::stricmp(str, "Light")==0)
        return WEIGHT_LIGHT;
    else if(CMisc::stricmp(str, "Medium")==0 || CMisc::stricmp(str, "Normal")==0 || CMisc::stricmp(str, "Roman")==0)
        return WEIGHT_MEDIUM;
    else if(CMisc::stricmp(str, "Regular")==0)
        return WEIGHT_MEDIUM; // WEIGHT_REGULAR;
    else if(CMisc::stricmp(str, "Demi")==0)
        return WEIGHT_DEMI;
    else if(CMisc::stricmp(str, "SemiBold")==0)
        return WEIGHT_SEMI_BOLD;
    else if(CMisc::stricmp(str, "DemiBold")==0)
        return WEIGHT_DEMI_BOLD;
    else if(CMisc::stricmp(str, "Thin")==0)
        return WEIGHT_THIN;
    else if(CMisc::stricmp(str, "Book")==0)
        return WEIGHT_BOOK;
    else
        return WEIGHT_MEDIUM; // WEIGHT_UNKNOWN;
}

CFontEngine::EWidth CFontEngine::strToWidth(const QString &str)
{
    if(str.isEmpty())
        return WIDTH_NORMAL; // WIDTH_UNKNOWN;
    else if(str.contains("UltraCondensed", false))
        return WIDTH_ULTRA_CONDENSED;
    else if(str.contains("ExtraCondensed", false))
        return WIDTH_EXTRA_CONDENSED;
    else if(str.contains("SemiCondensed", false))
        return WIDTH_SEMI_CONDENSED;
    else if(str.contains("Condensed", false))
        return WIDTH_CONDENSED;
    else if(str.contains("SemiExpanded", false))
        return WIDTH_SEMI_EXPANDED;
    else if(str.contains("UltraExpanded", false))
        return WIDTH_ULTRA_EXPANDED;
    else if(str.contains("ExtraExpanded", false))
        return WIDTH_EXTRA_EXPANDED;
    else if(str.contains("Expanded", false))
        return WIDTH_EXPANDED;
    else
        return WIDTH_NORMAL;
}

QString CFontEngine::createName(const QString &file, bool force)
{
    KFI_DBUG << "createName(" << file << ')' << endl;
    QString name;
    int     face=0,
            numFaces=0;

    do
    {
        if(openFont(file, NAME, force, face))
        {
            numFaces=getNumFaces();
            if(face>0)
                name.append(", ");
            name.append(getFullName());
            closeFont();
        }
    }
    while(++face<numFaces);

    return name;
}

#ifdef HAVE_FT_CACHE
static bool getCharMap(FT_Face &face, const QString &str)
{
    int cm;

    for(cm=0; cm<face->num_charmaps; cm++)
    {
        unsigned int ch;
        bool         found=true;

        FT_Set_Charmap(face, face->charmaps[cm]);
		unsigned int strLength( str.length());
        for(ch=0; ch< strLength && found; ch++)
            if(FT_Get_Char_Index(face, str[ch].unicode())==0)
                found=false;

        if(found)
            return true;
    }
    return false;
}

static void drawText(QPainter &painter, int x, int y, int width, const QString &str)
{
    QString s(str);
    bool    addedElipses=false;

    width-=x*2;
    while(s.length()>3 && painter.fontMetrics().size(0, s).width()>width)
    {
        if(!addedElipses)
        {
            s.remove(s.length()-2, 2);
            s.append("...");
            addedElipses=true;
        }
        else
            s.remove(s.length()-4, 1);
    }
    painter.drawText(x, y, s);
}

QString CFontEngine::getPreviewString()
{
    KConfig cfg(CGlobal::uiCfgFile());

    cfg.setGroup(KFI_PREVIEW_GROUP);

    QString str(cfg.readEntry(KFI_PREVIEW_STRING_KEY));

    return str.isEmpty() ? i18n("A sentence that uses all of the letters of the alphabet",
                                   "The quick brown fox jumps over the lazy dog")
                         : str;
}

void CFontEngine::setPreviewString(const QString &str)
{
    KConfig cfg(CGlobal::uiCfgFile());

    cfg.setGroup(KFI_PREVIEW_GROUP);
    cfg.writeEntry(KFI_PREVIEW_STRING_KEY, str);
}

#define FONT_CHAR_SIZE_MOD 0.75

void CFontEngine::createPreview(int width, int height, QPixmap &pix, int faceNo, int fSize, bool thumb, bool waterfall)
{
    static const int constWaterFallSizes[]={ 12, 18, 24, 36, 48, 60, 72, 84, 96, 108, 120, 0 };

    if(!isScaleable())
    {
        waterfall=false;
        fSize=-1;
    }

    FT_Size          size;
    FT_Face          face;
#if KFI_FT_IS_GE(2, 1, 8)
    FTC_ScalerRec    scaler;
    FTC_ImageTypeRec font;
#else
    FTC_Image_Desc   font;
#endif
    bool             isBitmap=isABitmap(itsType);
    int              fontSize=waterfall ? constWaterFallSizes[0] : fSize<0 || thumb ? 28 : fSize,
                     offset=4,
                     fontHeight;

    if(thumb && (width!=height || width>128))
        thumb=false;

    if(thumb)
    {
        if(height<=32)
        {
            offset=2;
            fontSize=(height-(offset*2))-2;
        }
        else
        {
            offset=3;
            fontSize=((height-(offset*3))-6)/2;
        }
    }

    fontHeight=isBitmap ? itsPixelSize : thumb ? fontSize : point2Pixel(fontSize);

#if KFI_FT_IS_GE(2, 1, 8)
    font.face_id=getId(itsPath, faceNo);
    font.width=font.height=fontHeight;
    font.flags=FT_LOAD_DEFAULT;
    scaler.face_id=font.face_id;
    scaler.width=scaler.height=font.width;
    scaler.pixel=1;
#else
    font.font.face_id=getId(itsPath, faceNo);
    font.font.pix_width=font.font.pix_height=fontHeight;
    font.image_type=ftc_image_grays;
#endif

    FT_F26Dot6 startX=offset,
               startY=offset+fontHeight,
               x=startX,
               y=startY;

    pix.resize(width, height);
    width-=offset;
    height-=offset;
    pix.fill(Qt::white);

    QPainter painter(&pix);

    if(!thumb)
    {
        QString name(itsFullName),
                info;
        QFont   title(KGlobalSettings::generalFont());

        if(isBitmap)
        {
            int pos=name.findRev('(');

            info=name.mid(pos);
            name=name.left(pos);
        }

        if(!itsFoundry.isEmpty() && constDefaultFoundry!=itsFoundry)
            name=i18n("%2 [%1]").arg(itsFoundry).arg(name);

        if(!isBitmap && !waterfall && fSize>0)
            name=i18n("%3,  %1pt / %2pt").arg(fSize).arg((int)(fSize*FONT_CHAR_SIZE_MOD)).arg(name);

        // title.setPixelSize(12);
        painter.setFont(title);
        painter.setPen(Qt::black);
        y=painter.fontMetrics().height();
        drawText(painter, x, y, width, name);

        if(isBitmap)
        {
            y+=2+painter.fontMetrics().height();
            drawText(painter, x, y, width, info);
        }

        y+=4;
        painter.drawLine(offset, y, width-offset, y);
        y+=2;
        y+=startY;
    }

#if KFI_FT_IS_GE(2, 1, 8)
    if(!FTC_Manager_LookupFace(itsFt.cacheManager, scaler.face_id, &face) &&
       !FTC_Manager_LookupSize(itsFt.cacheManager, &scaler, &size))
#else
    if(!FTC_Manager_Lookup_Size(itsFt.cacheManager, &(font.font), &face, &size))
#endif
    {
        FT_F26Dot6 stepY=size->metrics.y_ppem+offset;

        if(thumb)
        {
            QString str(i18n("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));

            if(getCharMap(face, str))
            {
                unsigned int ch;
				unsigned int strLength(str.length());
                for(ch=0; ch<strLength; ++ch)
                    if(drawGlyph(pix, font, FT_Get_Char_Index(face, str[ch].unicode()),  x, y, width, height, startX,
                                 stepY))
                        break;
            }
            else
                for(int i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
                    if(drawGlyph(pix, font, i, x, y, width, height, startX, stepY))
                        break;
        }
        else
        {
            QString  quote(getPreviewString());
            bool     foundCmap=getCharMap(face, quote);

            if(foundCmap)
            {
                unsigned int ch;

                if(waterfall)
                {
                    painter.setFont(KGlobalSettings::generalFont());
                    painter.setPen(Qt::black);

                    for(int s=0; constWaterFallSizes[s]; ++s)
                    {
                        QString txt(i18n("%1pt ").arg(constWaterFallSizes[s]));
                        bool    ok=true;

                        drawText(painter, x, y, width, txt);
                        x+=painter.fontMetrics().width(txt);

                        if(s)
                        {
#if KFI_FT_IS_GE(2, 1, 8)
                            font.width=font.height=point2Pixel(constWaterFallSizes[s]);
                            scaler.width=scaler.height=font.width;
#else
                            font.font.pix_width=font.font.pix_height=point2Pixel(constWaterFallSizes[s]);
#endif

#if KFI_FT_IS_GE(2, 1, 8)
                            ok=(!FTC_Manager_LookupFace(itsFt.cacheManager, scaler.face_id, &face) &&
                                !FTC_Manager_LookupSize(itsFt.cacheManager, &scaler, &size));
#else
                            ok=!FTC_Manager_Lookup_Size(itsFt.cacheManager, &(font.font), &face, &size);
#endif
                            stepY=size->metrics.y_ppem+offset;
                        }
                        if(ok)
                        {
                            for(ch=0; ch<quote.length(); ++ch)
                                if(drawGlyph(pix, font, FT_Get_Char_Index(face, quote[ch].unicode()), x, y, width, height,
                                             startX, stepY, false))
                                    break;
                            if(y>=height)
                                break;
                            x=startX;
                            y+=stepY*2;
                        }
                    }
                }
                else
                    for(ch=0; ch<quote.length(); ++ch)
                        if(drawGlyph(pix, font, FT_Get_Char_Index(face, quote[ch].unicode()), x, y, width, height, startX,
                                     stepY))
                            break;
            }

            if(!isBitmap)
#if KFI_FT_IS_GE(2, 1, 8)
            {
                font.width=font.height=point2Pixel((int)(fontSize*FONT_CHAR_SIZE_MOD));
                scaler.width=scaler.height=font.width;
            }
#else
                font.font.pix_width=font.font.pix_height=point2Pixel((int)(fontSize*0.75));
#endif

            if(y<height &&
#if KFI_FT_IS_GE(2, 1, 8)
               !FTC_Manager_LookupSize(itsFt.cacheManager, &scaler, &size))
#else
               !FTC_Manager_Lookup_Size(itsFt.cacheManager, &(font.font), &face, &size))
#endif
            {
                stepY=size->metrics.y_ppem+offset;

                if(foundCmap)
                {
                    if(x!=startX)
                    {
                        y+=stepY;
                        x=startX;
                    }

#if KFI_FT_IS_GE(2, 1, 8)
                    y+=font.height;
#else
                    y+=font.font.pix_height;
#endif
                }

                for(int i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
                    if(drawGlyph(pix, font, i, x, y, width, height, startX, stepY))
                        break;
            }
        }
    }
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TrueType, Type1, and Speedo
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void removeSymbols(QString &str)
{
    str.replace(QRegExp("[\\-\\[\\]()]"), " ");

    int   len=str.length();
    QChar space(' ');

    for(int c=0; c<len; ++c)
        if(str[c].unicode()<0x20 || str[c].unicode()>0x7E)
            str[c]=space;

    str=str.simplifyWhiteSpace();
    str=str.stripWhiteSpace();
}

static void removeString(QString &str, const QString &remove)
{
    static const QChar space(' '),
                       dash('-');
    int                pos=str.find(remove, 0, false);
    unsigned int       slen=remove.length();

    if((0==pos || (pos>0 && (space==str[pos-1] || dash==str[pos-1]))) &&
       (str.length()<=pos+slen || space==str[pos+slen] || dash==str[pos+slen]))
        str.remove(pos, slen);
}

void CFontEngine::createAddStyle()
{
    //
    // Get the extra style information. FreeType has a "style_name" field - this contains the info we want But may
    // also contain "Bold", "Italic", etc - these aren't really needed, so will be removed.
    if(itsFt.open)
    {
        itsAddStyle=itsFt.face->style_name;

        removeString(itsAddStyle, CFontEngine::weightStr(itsWeight));
        if(ITALIC_NONE!=itsItalic)
            removeString(itsAddStyle, ITALIC_ITALIC==itsItalic ? "Italic" : constOblique);
        removeString(itsAddStyle, constSlanted);
        removeString(itsAddStyle, CFontEngine::widthStr(itsWidth));
        removeString(itsAddStyle, "Cond");  // Some fonts just have Cond and not Condensed!
        removeString(itsAddStyle, "Regular");
        removeSymbols(itsAddStyle);
    }
    else
        itsAddStyle=QString::null;
}

static bool lookupName(FT_Face face, int nid, int pid, int eid, FT_SfntName *nameReturn)
{
    int n = FT_Get_Sfnt_Name_Count(face);

    if(n>0)
    {
        int         i;
        FT_SfntName name;

        for(i=0; i<n; i++)
            if(0==FT_Get_Sfnt_Name(face, i, &name) && name.name_id == nid && name.platform_id == pid &&
               (eid < 0 || name.encoding_id == eid))
            {
                switch(name.platform_id)
                {
                    case TT_PLATFORM_APPLE_UNICODE:
                    case TT_PLATFORM_MACINTOSH:
                        if(name.language_id != TT_MAC_LANGID_ENGLISH)
                            continue;
                        break;
                    case TT_PLATFORM_MICROSOFT:
                        if(name.language_id != TT_MS_LANGID_ENGLISH_UNITED_STATES &&
                           name.language_id != TT_MS_LANGID_ENGLISH_UNITED_KINGDOM)
                            continue;
                        break;
                    default:
                        continue;
                }

                if(name.string_len > 0)
                {
                    *nameReturn = name;
                    return true;
                }
            }
    }

    return false;
}

static QCString getName(FT_Face face, int nid)
{
    FT_SfntName name;
    QCString    str;

    if(lookupName(face, nid, TT_PLATFORM_MICROSOFT, TT_MS_ID_UNICODE_CS, &name) ||
       lookupName(face, nid, TT_PLATFORM_APPLE_UNICODE, -1, &name))
        for(unsigned int i=0; i < name.string_len / 2; i++)
            str+=0 == name.string[2*i] ? name.string[(2*i)+1] : '_';
    else if(lookupName(face, nid, TT_PLATFORM_MACINTOSH, TT_MAC_ID_ROMAN, &name)) // Pretend that Apple Roman is ISO 8859-1
        for(unsigned int i=0; i < name.string_len; i++)
            str+=name.string[i];

    return str;
}

static const char * getFoundry(const char *notice, bool retNull=false)
{
    struct Map
    {
        const char *noticeStr,
                   *foundry;
    };

    static const Map map[]=   // These are (mainly) taken from type1inst
    {
        { "Bigelow",                            "B&H"},
        { "Adobe",                              "Adobe"},
        { "Bitstream",                          "Bitstream"},
        { "Monotype",                           "Monotype"},
        { "Linotype",                           "Linotype"},
        { "LINOTYPE-HELL",                      "Linotype"},
        { "IBM",                                "IBM"},
        { "URW",                                "URW"},
        { "International Typeface Corporation", "ITC"},
        { "Tiro Typeworks",                     "Tiro"},
        { "XFree86",                            "XFree86"},
        { "Microsoft",                          "Microsoft"},
        { "Omega",                              "Omega"},
        { "Font21",                             "Hwan"},
        { "HanYang System",                     "Hanyang"},
        { "Richard Mitchell",                   "Mitchell" },
        { "Doug Miles",                         "Miles" },
        { "Hank Gillette",                      "Gillette" },
        { "Three Islands Press",                "3ip" },
        { "MacroMind",                          "Macromind" },
        { "MWSoft",                             "MWSoft" },
        { "Digiteyes Multimedia",               "DigitEyes" },
        { "ZSoft",                              "ZSoft" },
        { "Title Wave",                         "Titlewave" },
        { "Southern Software",                  "Southern" },
        { "Reasonable Solutions",               "Reasonable" },
        { "David Rakowski",                     "Rakowski" },
        { "D. Rakowski",                        "Rakowski" },
        { "S. G. Moye",                         "Moye" },
        { "S.G. Moye",                          "Moye" },
        { "Andrew s. Meit",                     "Meit" },
        { "A.S.Meit",                           "Meit" },
        { "Hershey",                            "Hershey" },
        { "FontBank",                           "FontBank" },
        { "A. Carr",                            "Carr" },
        { "Brendel Informatik",                 "Brendel" },
        { "Jonathan Brecher",                   "Brecher" },
        { "SoftMaker",                          "Softmaker" },
        { "LETRASET",                           "Letraset" },
        { "Corel Corp",                         "Corel"},
        { "PUBLISHERS PARADISE",                "Paradise" },
        { "Publishers Paradise",                "Paradise" },
        { "Allied Corporation",                 "Allied" },
        { NULL,                                 NULL }
    };

    const Map *entry;

    if(notice)
        for(entry=map; NULL!=entry->foundry; entry++)
            if(strstr(notice, entry->noticeStr)!=NULL)
                return entry->foundry;

    return retNull ? NULL : constDefaultFoundry;
}

static const char * getFoundry(const FT_Face face, TT_OS2 *os2)
{
    struct Map
    {
        const char *vendorId,
                   *foundry;
    };

    static const int constVendLen=4;

    // These are taken from ttmkfdir
    // Removed any commas - StarOffice doesn't like these...
    // Shortened quite a few entires to help with StarOffice
    static const Map map[]=
    {
        { "ADBE", "Adobe"},
        { "AGFA", "Agfa"},
        { "ALTS", "Altsys"},
        { "APPL", "Apple"},
        { "ARPH", "Arphic"},
        { "ATEC", "Alltype"},
        { "B&H",  "B&H"},
        { "BITS", "Bitstream"},
        { "CANO", "Cannon"},
        { "DYNA", "DynaLab"},
        { "EPSN", "Epson"},
        { "FJ",   "Fujitsu"},
        { "IBM",  "IBM"},
        { "ITC",  "ITC"},
        { "IMPR", "Impress"},
        { "LARA", "Larabie"},
        { "LEAF", "Interleaf"},
        { "LETR", "letraset"},
        { "LINO", "Linotype"},
        { "MACR", "Macromedia"},
        { "MONO", "Monotype"},
        { "MS",   "Microsoft"},
        { "MT",   "Monotype"},
        { "NEC",  "NEC"},
        { "PARA", "ParaType"},
        { "QMSI", "QMS"},
        { "RICO", "Ricoh"},
        { "URW",  "URW"},
        { "Y&Y" , "Z&Y"},
        { "2REB", "2Rebels"}, 
        { "3IP" , "3ip"},
        { "ABC" , "Altek Inst"},
        { "ACUT", "AcuteType"},
        { "AOP" , "Art Of Penguin"},
        { "AZLS", "Azalea"},
        { "BARS", "CIA UK"},
        { "BERT", "Berthold"},
        { "BITM", "Bitmap Soft"},
        { "BLAH", "MisterBlas"},
        { "BOYB", "I Frances"},
        { "BRTC", "Bear Rock"},
        { "BWFW", "B/W Fontworks"},
        { "C&C",  "Carter&Cone"},
        { "CAK" , "Pluginfonts"},
        { "CASL", "H W Caslon"},
        { "COOL", "CoolFonts"},
        { "CTDL", "ChinaType"},
        { "DAMA", "DM Ltd"},
        { "DS"  , "Dainippon"},
        { "DSCI", "Design Science"},
        { "DTC",  "Digital Typeface"},
        { "DTPS", "DTP Software"},
        { "DUXB", "Duxbury"},
        { "ECF" , "Emerald City"},
        { "EDGE", "Rivers Edge"},
        { "EFF" , "Electronic"},
        { "EFNT", "E Fonts"},
        { "ELSE", "Elseware"},
        { "ERAM", "Eraman"},
        { "ESIG", "E Signature"},
        { "FBI",  "Font Bureau"},
        { "FCAB", "Font Cabinet"},
        { "FONT", "Font Source"},
        { "FS"  , "Formula"},
        { "FSE" , "Font Source Europe"},
        { "FSI" , "FSI GmbH"},
        { "FTFT", "FontFont"},
        { "FWRE", "Fontware"},
        { "GALA", "Galapagos"},
        { "GD"  , "GDFonts"},
        { "GLYF", "Glyph Systems"},
        { "GPI",  "Gamma Productions"},
        { "HY",   "HanYang System"},
        { "HILL", "Hill Systems"},
        { "HOUS", "House Industries"},
        { "HP",   "HP"},
        { "IDEE", "IDEE"},
        { "IDF",  "Digital Fonts"},
        { "ILP",  "Indigenous Lang"},
        { "ITF" , "Int Type Founders"},
        { "KATF", "Kingsley"},
        { "LANS", "Lanston"},
        { "LGX" , "Logix Research"},
        { "LING", "Linguists"},
        { "LP",   "LetterPerfect"},
        { "LTRX", "Lighttracks"},
        { "MC"  , "Cerajewski"},
        { "MILL", "Millan"},
        { "MJ"  , "Majus"},
        { "MLGC", "Micrologic"},
        { "MSCR", "Majus"},
        { "MTY" , "Motoya"},
        { "MUTF", "CACHE"},
        { "NB"  , "No Bodoni"},
        { "NDTC", "Neufville Digital"},
        { "NIS" , "NIS"},
        { "ORBI", "Orbit"},
        { "P22" , "P22"},
        { "PDWX", "Parsons Design"},
        { "PF"  , "Phils Fonts"},
        { "PRFS", "Production"},
        { "RKFN", "R K Fonts"},
        { "ROBO", "Buro Petr"},
        { "RUDY", "Rudyn Fluffy"},
        { "SAX" , "SAX gmbh"},
        { "SEAN", "The FontSite"},
        { "SFI" , "Software Friends"},
        { "SFUN", "Soft Union"},
        { "SG"  , "Scooter Graphics"},
        { "SIG" , "Signature"},
        { "SKZ" , "Celtic Ladys"},
        { "SN"  , "SourceNet"},
        { "SOHO", "SoftHorizons"},
        { "SOS" , "Standing Ovations"},
        { "STF" , "Brian Sooy"},
        { "STON", "ZHUHAI"},
        { "SUNW", "Sunwalk"},
        { "SWFT", "Swfte"},
        { "SYN" , "SynFonts"},
        { "TDR" , "Tansin A Darcos"},
        { "TF"  , "Treacyfaces"},
        { "TILD", "SIA Tilde"},
        { "TPTC", "Test Pilot"},
        { "TR"  , "Type Revivals"},
        { "TS"  , "TamilSoft"},
        { "UA"  , "UnAuthorized Type"},
        { "VKP" , "Vijay K Patel"},
        { "VLKF", "Visualogik"},
        { "VOG" , "Martin Vogel"},
        { "ZEGR", "Zebra Font Facit"},
        { "ZETA", "Tangram Studio"},
        { "ZSFT", "ZSoft"},
        { NULL  ,  NULL}
    };

    static char vendor[constVendLen+1];

    vendor[0]='\0';

    if(NULL!=os2 && 0xFFFF!=os2->version)
    {
        const Map *entry;
        char      vend[constVendLen+1];

        strncpy(vendor, (const char *)(os2->achVendID), constVendLen);
        vendor[constVendLen]='\0';

        for(int i=0; i<constVendLen; ++i)
            vend[i]=toupper(vendor[i]);

        for(entry=map; NULL!=entry->vendorId; entry++)
        {
            unsigned int len=strlen(entry->vendorId);

            if(0==memcmp(entry->vendorId, vend, len))
            {
                bool ok=true;

                for(int i=len; i<constVendLen && ok; i++)
                    if(vend[i]!=' ' && entry->vendorId[i]!='\0')
                        ok=false;

                if(ok)
                    return entry->foundry;
            }
        }
    }
                
    const char *foundry=NULL;

    if(!foundry)
        foundry=getFoundry(getName(face, TT_NAME_ID_TRADEMARK), true);

    if(!foundry)
        foundry=getFoundry(getName(face, TT_NAME_ID_MANUFACTURER), true);

    if(!foundry && vendor[0] && !isspace(vendor[0]) && '-'!=vendor[0])  // Some fonts have a totally blank vendor field
    {
       int i;

        // Remove any dashes...
        for(int i=constVendLen-1; i>0; i--)
            if('-'==vendor[i])
                vendor[i]=' ';

        // Strip any trailing whitepace
        for(i=constVendLen-1; i>0; i--)
            if(isspace(vendor[i]))
               vendor[i]='\0';
            else
               break;

        foundry=vendor;
    }

    return foundry ? foundry : constDefaultFoundry;
}

static CFontEngine::EItalic checkItalic(CFontEngine::EItalic it, const QString &full)
{
    return (CFontEngine::ITALIC_ITALIC==it && (-1!=full.find(constOblique) || -1!=full.find(constSlanted)))
           ? CFontEngine::ITALIC_OBLIQUE
           : it;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TYPE 1
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontT1(const QString &file, unsigned short mask)
{
    bool status=FT_New_Face(itsFt.library, QFile::encodeName(file), 0, &itsFt.face) ? false : true;

    if(status)
        itsFt.open=true;

    if(mask&NAME && status)
    {
        PS_FontInfoRec t1info;

        if(0==FT_Get_PS_Font_Info(itsFt.face, &t1info))
        {
            itsFullName=t1info.full_name;

            if(NAME==mask && !itsFullName.isEmpty())
            {
                removeSymbols(itsFullName);
                return status;
            }

            itsFamily=t1info.family_name;

            if(itsFamily.isEmpty())
                if(itsFullName.isEmpty())
                    itsFamily=itsFullName=FT_Get_Postscript_Name(itsFt.face);
                else
                    itsFamily=itsFullName;
            else
                if(itsFullName.isEmpty())
                        itsFullName=itsFamily;

                if(itsFullName.isEmpty())
                    status=false;   // Hmm... couldn't find any of the names!
        }
        else
            status=false;

        if(status)
        {
            removeSymbols(itsFullName);

            if(NAME!=mask)
            {
                removeSymbols(itsFamily);
                setPsNameFt();
                itsWeight=strToWeight(t1info.weight);
                itsItalic=t1info.italic_angle <= -4 || t1info.italic_angle >= 4 ? ITALIC_ITALIC : ITALIC_NONE;
                itsWidth=strToWidth(itsFullName);
                itsItalic=checkItalic(itsItalic, itsFullName);

                if(mask&XLFD)
                {
                    itsSpacing=t1info.is_fixed_pitch ? SPACING_MONOSPACED : SPACING_PROPORTIONAL;
                    itsFoundry=::getFoundry(t1info.notice);
                    createAddStyle();
                }
            }
        }
    }

    if(!status)
        closeFaceFt();

    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    AFM
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontAfm(const QString &file)
{
    QFile f(file);
    bool  foundName=false,
          foundFamily=false,
          foundPs=false;

    if(f.open(IO_ReadOnly))
    {
        QTextStream stream(&f);
        QString     line;
        bool        inMetrics=false;

        while(!stream.atEnd())
        {
            line=stream.readLine();
            line=line.simplifyWhiteSpace();

            if(inMetrics)
            {
                if(0==line.find("FontName "))
                {
                    itsPsName=line.mid(9);
                    foundPs=true;
                }
                else if(0==line.find("FullName "))
                {
                    itsFullName=line.mid(9);
                    itsWidth=strToWidth(itsFullName);
                    foundName=true;
                }
                else if(0==line.find("FamilyName "))
                {
                    itsFamily=line.mid(11);
                    foundFamily=true;
                }
                else if(0==line.find("Weight "))
                    itsWeight=strToWeight(line.mid(7).latin1());
                else if(0==line.find("ItalicAngle "))
                    itsItalic=0.0f==line.mid(12).toFloat() ? ITALIC_NONE : ITALIC_ITALIC;
                else if(0==line.find("IsFixedPitch "))
                    itsSpacing=0==line.mid(13).find("false", 0, false) ? SPACING_PROPORTIONAL : SPACING_MONOSPACED;
                else if(0==line.find("Notice "))
                    itsFoundry=::getFoundry(line.mid(7).latin1());
                else if(0==line.find("StartCharMetrics"))
                    break;
                itsItalic=checkItalic(itsItalic, itsFullName);
            }
            else
                if(0==line.find("StartFontMetrics"))
                    inMetrics=true;
        };
        f.close();

        if(!foundFamily && foundName)
        {
            itsFamily=itsFullName;
            foundFamily=true;
        }

        if(foundName)
            itsItalic=checkItalic(itsItalic, itsFullName);
        createAddStyle();
    }

    return foundPs && foundName && foundFamily;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TRUE TYPE
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontTT(const QString &file, unsigned short mask)
{
    bool status=FT_New_Face(itsFt.library, QFile::encodeName(file), itsFaceIndex, &itsFt.face) ? false : true;

    if(status)
        itsFt.open=true;

    if(mask&NAME && status)
    {
        itsFullName=getName(itsFt.face, TT_NAME_ID_FULL_NAME);

        if(NAME==mask && !itsFullName.isEmpty())
        {
            removeSymbols(itsFullName);
            return status;
        }

        itsFamily=getName(itsFt.face, TT_NAME_ID_FONT_FAMILY);

        if(itsFamily.isEmpty())
            if(itsFullName.isEmpty())
                itsFamily=itsFullName=FT_Get_Postscript_Name(itsFt.face);
            else
                itsFamily=itsFullName;
        else
            if(itsFullName.isEmpty())
                itsFullName=itsFamily;

        if(itsFullName.isEmpty())
            status=false;   // Hmm... couldn't find any of the names!
        else
        {
            removeSymbols(itsFullName);

            if(NAME!=mask)
            {
                TT_Postscript *post=NULL;
                TT_OS2        *os2=NULL;
                TT_Header     *head=NULL;
                bool          gotItalic=false;
                
                removeSymbols(itsFamily);
                setPsNameFt();
    
                if(NULL==(os2=(TT_OS2 *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2)) || 0xFFFF==os2->version)
                {
                    itsWidth=WIDTH_UNKNOWN;
                    itsWeight=WEIGHT_UNKNOWN;
                }
                else
                {
                    itsWeight=mapWeightTT(os2->usWeightClass);
                    itsWidth=mapWidthTT(os2->usWidthClass);
                    if(WEIGHT_UNKNOWN==itsWeight && os2->fsSelection&(1 << 5))
                        itsWeight=WEIGHT_BOLD;
                    itsItalic=os2->fsSelection&(1 << 0) ? ITALIC_ITALIC : ITALIC_NONE;
                    gotItalic=true;
                }
    
                if(WEIGHT_UNKNOWN==itsWeight && NULL!=(head=(TT_Header *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head)) &&
                   head->Mac_Style & 1)
                    itsWeight=WEIGHT_BOLD;
                if(WIDTH_UNKNOWN==itsWidth)
                    itsWidth=WIDTH_NORMAL;
    
                if(!gotItalic && (head!=NULL || NULL!=(head=(TT_Header *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head))))
                {
                    gotItalic=true;
                    itsItalic=head->Mac_Style & 2 ? ITALIC_ITALIC: ITALIC_NONE; 
                }
    
                if(!gotItalic && NULL!=(post=(TT_Postscript *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post)))
                {
                    struct TFixed
                    {
                        TFixed(unsigned long v) : upper(v>>16), lower(v&0xFFFF) {}
    
                        short upper,
                            lower;
    
                        float value() { return upper+(lower/65536.0); }
                    };
    
                    gotItalic=true;
                    itsItalic=0.0f==((TFixed)post->italicAngle).value() ? ITALIC_NONE : ITALIC_ITALIC;
                }
    
                if(WEIGHT_UNKNOWN==itsWeight)
                    itsWeight=WEIGHT_MEDIUM;
    
                itsItalic=checkItalic(itsItalic, itsFullName);
    
                if(mask&XLFD)
                {
                    if((NULL!=post || NULL!=(post=(TT_Postscript *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post))) &&
                       post->isFixedPitch)
                    {
                        TT_HoriHeader *hhea=NULL;
    
                        if(NULL!=(hhea=(TT_HoriHeader *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_hhea)) &&
                        hhea->min_Left_Side_Bearing >= 0 && hhea->xMax_Extent <= hhea->advance_Width_Max)
                            itsSpacing=SPACING_CHARCELL;
                        else
                            itsSpacing=SPACING_MONOSPACED;
                    }
                    else
                        itsSpacing=SPACING_PROPORTIONAL;
    
                    itsFoundry=::getFoundry(itsFt.face, os2);
                    createAddStyle();
                }
            }
        }
    }

    if(!status)
        closeFaceFt();

    return status;
}

CFontEngine::EWeight CFontEngine::mapWeightTT(FT_UShort os2Weight)
{
    //
    // MS's specs defines calues 100, 200, etc - whareas Apple's defines 1, 2, ...
    FT_UShort weight=(os2Weight>0 && os2Weight<100) ? os2Weight*100 : os2Weight;

    enum ETtfWeight
    {
        TTF_WEIGHT_UNKNOWN    = 0,
        TTF_WEIGHT_THIN       = 100 +50,
        TTF_WEIGHT_EXTRALIGHT = 200 +50,
        TTF_WEIGHT_LIGHT      = 300 +50,
        TTF_WEIGHT_NORMAL     = 400 +50,
        TTF_WEIGHT_MEDIUM     = 500 +50,
        TTF_WEIGHT_SEMIBOLD   = 600 +50,
        TTF_WEIGHT_BOLD       = 700 +50,
        TTF_WEIGHT_EXTRABOLD  = 800 +50,
        TTF_WEIGHT_BLACK      = 900 +50
    };

    if(weight<TTF_WEIGHT_THIN)
        return WEIGHT_THIN;
    else if(weight<TTF_WEIGHT_EXTRALIGHT)
        return WEIGHT_EXTRA_LIGHT;
    else if(weight<TTF_WEIGHT_LIGHT)
        return WEIGHT_LIGHT;
    else if(/*weight<TTF_WEIGHT_NORMAL || */ weight<TTF_WEIGHT_MEDIUM)
        return WEIGHT_MEDIUM;
    else if(weight<TTF_WEIGHT_SEMIBOLD)
        return WEIGHT_SEMI_BOLD;
    else if(weight<TTF_WEIGHT_BOLD)
        return WEIGHT_BOLD;
    else if(weight<TTF_WEIGHT_EXTRABOLD)
        return WEIGHT_EXTRA_BOLD;
    else if(weight<TTF_WEIGHT_BLACK)
        return WEIGHT_BLACK;
    else
        return WEIGHT_UNKNOWN;
}

CFontEngine::EWidth CFontEngine::mapWidthTT(FT_UShort os2Width)
{
    enum ETtfWidth
    {
        TTF_WIDTH_ULTRA_CONDENSED = 1,
        TTF_WIDTH_EXTRA_CONDENSED = 2,
        TTF_WIDTH_CONDENSED       = 3,
        TTF_WIDTH_SEMI_CONDENSED  = 4,
        TTF_WIDTH_NORMAL          = 5,
        TTF_WIDTH_SEMI_EXPANDED   = 6,
        TTF_WIDTH_EXPANDED        = 7,
        TTF_WIDTH_EXTRA_EXPANDED  = 8,
        TTF_WIDTH_ULTRA_EXPANDED  = 9
    };

    switch(os2Width)
    {
        case TTF_WIDTH_ULTRA_CONDENSED:
            return WIDTH_ULTRA_CONDENSED;
        case TTF_WIDTH_EXTRA_CONDENSED:
            return WIDTH_EXTRA_CONDENSED;
        case TTF_WIDTH_CONDENSED:
            return WIDTH_CONDENSED;
        case TTF_WIDTH_SEMI_CONDENSED:
            return WIDTH_SEMI_CONDENSED;
        case TTF_WIDTH_NORMAL:
            return WIDTH_NORMAL;
        case TTF_WIDTH_SEMI_EXPANDED:
            return WIDTH_SEMI_EXPANDED;
        case TTF_WIDTH_EXPANDED:
            return WIDTH_EXPANDED;
        case TTF_WIDTH_EXTRA_EXPANDED:
            return WIDTH_EXTRA_EXPANDED;
        case TTF_WIDTH_ULTRA_EXPANDED:
            return WIDTH_ULTRA_EXPANDED;
        default:
            return WIDTH_UNKNOWN;
    }
}

void CFontEngine::closeFaceFt()
{
    if(itsFt.open)
    {
        FT_Done_Face(itsFt.face);
        itsFt.open=false;
    }
}

void CFontEngine::setPsNameFt()
{
    itsPsName=(FT_Get_Postscript_Name(itsFt.face));

    if(itsPsName.isEmpty())
    {
        itsPsName=itsFullName;

        // Must not start with a digit
        if(!itsPsName.isEmpty())
        {
            unsigned int ch,
                         ch2;

            if(itsPsName[0].isDigit())
                itsPsName[0]=itsPsName.local8Bit()[0]+('A'-'0');

            for(ch=1; ch<itsPsName.length(); ++ch)
                if('_'==itsPsName.local8Bit()[ch] || ' '==itsPsName.local8Bit()[ch])
                    for(ch2=ch; ch2<itsPsName.length()-1; ++ch2)
                        itsPsName[ch2]=itsPsName[ch2+1];
        }
    }
}

#ifndef HAVE_FONT_ENC
bool CFontEngine::has16BitEncodingFt(const QString &enc)
{
    if(enc=="jisx0208.1983-0" || enc=="jisx0201.1976-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_sjis) ? false : true;
    else if(enc=="gb2312.1980-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_gb2312) ? false : true;
    else if(enc=="big5.et-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_big5) ? false : true;
    else if(enc=="ksc5601.1987-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_wansung) ?  FT_Select_Charmap(itsFt.face, ft_encoding_johab) ? 
               false : true : true;
    else
        return false;
}

bool CFontEngine::has8BitEncodingFt(CEncodings::T8Bit *data)
{
    if(data)
    {
        if(data->load())
        {
            int cm;

            for(cm=0; cm<itsFt.face->num_charmaps; cm++)
            {
                static const int constMaxMissing=5;

                int ch,
                    missing=0;

                setCharmapFt(itsFt.face->charmaps[cm]);

                for(ch=0; ch<CEncodings::T8Bit::NUM_MAP_ENTRIES && missing<=constMaxMissing; ch++)
                    if(data->map[ch]>-1 && FT_Get_Char_Index(itsFt.face, data->map[ch])==0)
                        missing++;

                if(missing<=constMaxMissing)
                    return true;
            }
        }
    }

    return false;
}

QStringList CFontEngine::get8BitEncodingsFt()
{
    QStringList enc;

    // Do 8-bit encodings...
    CEncodings::T8Bit *enc8;

    for(enc8=CGlobal::enc().first8Bit(); enc8; enc8=CGlobal::enc().next8Bit())
        if(has8BitEncodingFt(enc8))
            enc.append(enc8->name);

    return enc;
}

#else

inline bool codeIgnored(int c)
{
    return (c) < 0x20 || (c >= 0x7F && c <= 0xA0) || c == 0xAD || c == 0xF71B;
}

bool CFontEngine::findCharMapFt(int type, int pid, int eid)
{
    FT_CharMap cmap = NULL;
    int        i;

    switch(type)
    {
        case FONT_ENCODING_TRUETYPE:  // specific cmap
            for(i=0; i<itsFt.face->num_charmaps; i++)
            {
                cmap=itsFt.face->charmaps[i];

                if(cmap->platform_id == pid && cmap->encoding_id == eid)
                    return FT_Set_Charmap(itsFt.face, cmap)==0 ? true : false;
            }
            break;
        case FONT_ENCODING_UNICODE:   // any Unicode cmap
            // prefer Microsoft Unicode 
            for(i=0; i<itsFt.face->num_charmaps; i++)
            {
                cmap = itsFt.face->charmaps[i];
                if(cmap->platform_id == TT_PLATFORM_MICROSOFT && cmap->encoding_id == TT_MS_ID_UNICODE_CS)
                    return FT_Set_Charmap(itsFt.face, cmap)==0 ? true : false;
            }
            break;   // CPD??? This means the following is NOT used???
            // Try Apple Unicode
            for(i=0; i<itsFt.face->num_charmaps; i++)
            {
                cmap = itsFt.face->charmaps[i];
                if(cmap->platform_id == TT_PLATFORM_APPLE_UNICODE)
                    return FT_Set_Charmap(itsFt.face, cmap)==0 ? true : false;
            }
            // ISO Unicode?
            for(i=0; i<itsFt.face->num_charmaps; i++)
            {
                cmap = itsFt.face->charmaps[i];
                if(cmap->platform_id == TT_PLATFORM_ISO)
                    return FT_Set_Charmap(itsFt.face, cmap)==0 ? true : false;
            }
            break;
        default:
            return false;
    }

    return false;
}

bool CFontEngine::checkEncodingFt(const QString &enc)
{
    static const float constBigEncodingFuzz = 0.02;

    FontEncPtr encoding=FontEncFind(enc.latin1(), NULL);   // CPD latin1 ???

    if(encoding)
    {
        FontMapPtr mapping; 

        /* An encoding is ``small'' if one of the following is true:
             - it is linear and has no more than 256 codepoints; or
             - it is a matrix encoding and has no more than one column.
       
           For small encodings using Unicode indices, we require perfect
           coverage except for codeIgnored and KOI-8 IBM-PC compatibility.

           For large encodings, we require coverage up to constBigEncodingFuzz.

           For encodings using PS names (currently Adobe Standard and
           Adobe Symbol only), we require perfect coverage. */

        if(FT_Has_PS_Glyph_Names(itsFt.face))
            for(mapping=encoding->mappings; mapping; mapping=mapping->next)
                if(FONT_ENCODING_POSTSCRIPT==mapping->type)
                {
                    if(encoding->row_size > 0)
                        for(int i = encoding->first; i < encoding->size; i++)
                            for(int j = encoding->first_col; j < encoding->row_size; j++)
                            {
                                char *name=FontEncName((i<<8) | j, mapping);
                                if(name && 0==FT_Get_Name_Index(itsFt.face, name))
                                    return false;
                            }
                    else
                        for(int i = encoding->first; i < encoding->size; i++)
                        {
                            char *name=FontEncName(i, mapping);
                            if(name && 0==FT_Get_Name_Index(itsFt.face, name))
                                return false;
                        }
                    return true;
                }

        for(mapping = encoding->mappings; mapping; mapping = mapping->next)
        {
            if(findCharMapFt(mapping->type, mapping->pid, mapping->eid))
            {
                int total=0,
                    failed=0;

                if(encoding->row_size > 0)
                {
                    int estimate = (encoding->size - encoding->first) * (encoding->row_size - encoding->first_col);

                    for(int i = encoding->first; i < encoding->size; i++)
                        for(int j = encoding->first_col; j < encoding->row_size; j++)
                        {
                            int c=FontEncRecode((i<<8) | j, mapping);

                            if(!codeIgnored(c))
                            {
                                if(0==FT_Get_Char_Index(itsFt.face, c))
                                    failed++;
                                total++;
                                if((encoding->size <= 1 && failed > 0) ||
                                   ((float)failed >= constBigEncodingFuzz * estimate))
                                    return false;
                            }
                        }

                    return (float)failed >= total * constBigEncodingFuzz ? false : true;
                }
                else
                {
                    int estimate = encoding->size - encoding->first;
                    // For the KOI8 encodings, we ignore the lack of linedrawing and pseudo-math characters
                    bool koi8=0==strncmp(encoding->name, "koi8-", 5) ? true : false;

                    for(int i = encoding->first; i < encoding->size; i++)
                    {
                        int c=FontEncRecode(i, mapping);

                        if(!codeIgnored(c) && !(koi8 && ((c >= 0x2200 && c < 0x2600) || c == 0x00b2)))
                        {
                            if(0==FT_Get_Char_Index(itsFt.face, c))
                                failed++;
                            total++;
                            if((encoding->size <= 256 && failed > 0) || ((float)failed >= constBigEncodingFuzz * estimate))
                                return false;
                        }
                    }

                    return (float)failed >= total * constBigEncodingFuzz ? false : true;
                }
            }
        }
    }

    return false;
}

bool CFontEngine::checkExtraEncodingFt(const QString &enc, bool found)
{
    if(enc==CEncodings::constUnicode)
    {
        if(findCharMapFt(FONT_ENCODING_UNICODE, -1, -1))
        {
            int num = 0,
                c;

            // Export as Unicode if there are at least 15 BMP characters that are not a space or ignored.
            for(c = 0x21; c<0x10000; c++)
                if(!codeIgnored(c) && FT_Get_Char_Index(itsFt.face, c)>0 && ++num>= 15)
                    return true;
        }
    }
    else if(enc==CEncodings::constTTSymbol)
    {
        if(findCharMapFt(FONT_ENCODING_TRUETYPE, TT_PLATFORM_MICROSOFT, TT_MS_ID_SYMBOL_CS))
            return true;
    }
    else if(enc==CEncodings::constT1Symbol)
        return !found && FT_Has_PS_Glyph_Names(itsFt.face) ? true : false;

    return false;
}

#endif

QStringList CFontEngine::getEncodingsFt()
{
    QStringList enc;

    // Check for symbol encoding...
    if(setCharmapSymbolFt())
        enc.append(itsType==TYPE_1 ? CEncodings::constT1Symbol : CEncodings::constTTSymbol);
    else
    {
#ifdef HAVE_FONT_ENC
        QStringList::ConstIterator it;
        bool                       found=false;

        for(it=CGlobal::enc().getList().begin(); it!=CGlobal::enc().getList().end(); ++it)
            if(checkEncodingFt(*it))
            {
                enc.append(*it);
                found=true;
            }

        for(it=CGlobal::enc().getExtraList().begin(); it!=CGlobal::enc().getExtraList().end(); ++it)
            if(checkExtraEncodingFt(*it, found))
            {
                enc.append(*it);
                found=true;
            }
#else
        // Add Unicode encoding...
        if(setCharmapUnicodeFt())
            enc.append(CEncodings::constUnicode);

        // Do 8-bit encodings...
        enc+=get8BitEncodingsFt();

        if(TRUE_TYPE==itsType || TT_COLLECTION==itsType || OPEN_TYPE==itsType)
        {
            // Do 16-bit encodings...
            CEncodings::T16Bit *enc16;

            for(enc16=CGlobal::enc().first16Bit(); enc16; enc16=CGlobal::enc().next16Bit())
                if(has16BitEncodingFt(enc16->name))
                    enc.append(enc16->name);
        }
#endif
    }

    if(0==enc.count())
        enc.append("iso8859-1");

    return enc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    SPEEDO
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontSpd(const QString &file, unsigned short mask)
{
    const int           constHeaderSize=420;
    const int           constShortNameOffset=298;
    const int           constShortNameNumBytes=16;
    const int           constSourceFontNameOffset=24;
    const int           constSourceFontNameNumBytes=70;
    const int           constClassificationOffset=265;
    const unsigned char constWidthMask=0x0f;
    const unsigned char constWeightMask=0xf0;
    const int           constWeightShift=4;
    const int           constItalicOffset=328;
    const int           constSpacingOffset=264;
    const int           constMonospaced=3;
    const int           constNoticeOffset=174;
    const int           constNoticeNumBytes=78;

    enum ESpeedoWidth
    {
        SPD_WIDTH_CONDENSED=4,
        SPD_WIDTH_SEMI_CONDENSED=6,
        SPD_WIDTH_NORMAL=8,
        SPD_WIDTH_SEMI_EXPANDED=10,
        SPD_WIDTH_EXPANDED=12
    };

    enum ESpeedoWeight
    {
        SPD_WEIGHT_UNKNOWN=0,
        SPD_WEIGHT_THIN,
        SPD_WEIGHT_ULTRA_LIGHT,
        SPD_WEIGHT_EXTRA_LIGHT,
        SPD_WEIGHT_LIGHT,
        SPD_WEIGHT_BOOK,
        SPD_WEIGHT_NORMAL,
        SPD_WEIGHT_MEDIUM,
        SPD_WEIGHT_SEMI_BOLD,
        SPD_WEIGHT_DEMI_BOLD,
        SPD_WEIGHT_BOLD,
        SPD_WEIGHT_EXTRA_BOLD,
        SPD_WEIGHT_ULTRA_BOLD,
        SPD_WEIGHT_HEAVY,
        SPD_WEIGHT_BLACK
    };

    bool          status=false;
    std::ifstream spd(QFile::encodeName(file));

    if(spd)
    {
        char hdr[constHeaderSize];

        spd.read(hdr, sizeof(char)*constHeaderSize);
        if(spd.good())
        {
            //
            // The first 4 bytes of s Speedo font are assumed to be:
            //    1. 'D' (or maybe 'd' ?)
            //    2. A number (usually 1 or 4)
            //    3. Decimal point
            //    4. A number (usually 0)
            if((hdr[0]=='D' || hdr[0]=='d') && isdigit(hdr[1]) && hdr[2]=='.' && isdigit(hdr[3]))
            {
                char shortName[constShortNameNumBytes+1];
                memcpy(shortName, &hdr[constShortNameOffset], constShortNameNumBytes);
                shortName[constShortNameNumBytes]='\0';
                itsFamily=shortName;

                char sourceName[constSourceFontNameNumBytes+1];
                memcpy(sourceName, &hdr[constSourceFontNameOffset], constSourceFontNameNumBytes);
                sourceName[constSourceFontNameNumBytes]='\0';
                itsFullName=sourceName;
                itsPsName=constNoPsName;
                status=true;

                removeSymbols(itsFamily);
                removeSymbols(itsFullName);

                if(mask&PROPERTIES)
                {
                    switch((hdr[constClassificationOffset]&constWeightMask)>>constWeightShift)
                    {
                        case SPD_WEIGHT_THIN:
                            itsWeight=WEIGHT_THIN;
                            break;
                        case SPD_WEIGHT_ULTRA_LIGHT:
                            itsWeight=WEIGHT_ULTRA_LIGHT;
                            break;
                        case SPD_WEIGHT_EXTRA_LIGHT:
                            itsWeight=WEIGHT_EXTRA_LIGHT;
                            break;
                        case SPD_WEIGHT_LIGHT:
                            itsWeight=WEIGHT_LIGHT;
                            break;
                        case SPD_WEIGHT_BOOK:
                            itsWeight=WEIGHT_BOOK;
                            break;
                        case SPD_WEIGHT_NORMAL:
                        case SPD_WEIGHT_MEDIUM:
                            itsWeight=WEIGHT_MEDIUM;
                            break;
                        case SPD_WEIGHT_SEMI_BOLD:
                            itsWeight=WEIGHT_SEMI_BOLD;
                            break;
                        case SPD_WEIGHT_DEMI_BOLD:
                            itsWeight=WEIGHT_DEMI_BOLD;
                            break;
                        case SPD_WEIGHT_BOLD:
                            itsWeight=WEIGHT_BOLD;
                            break;
                        case SPD_WEIGHT_EXTRA_BOLD:
                            itsWeight=WEIGHT_EXTRA_BOLD;
                            break;
                        case SPD_WEIGHT_ULTRA_BOLD:
                            itsWeight=WEIGHT_ULTRA_BOLD;
                            break;
                        case SPD_WEIGHT_HEAVY:
                            itsWeight=WEIGHT_HEAVY;
                            break;
                        case SPD_WEIGHT_BLACK:
                            itsWeight=WEIGHT_BLACK;
                            break;
                        case SPD_WEIGHT_UNKNOWN:
                        default:
                            itsWeight=WEIGHT_MEDIUM;
                    }

                    itsItalic=(0==(hdr[constItalicOffset]<<8 + hdr[constItalicOffset+1])) ? ITALIC_NONE : ITALIC_ITALIC;

                    switch(hdr[constClassificationOffset]&constWidthMask)
                    {
                        case SPD_WIDTH_CONDENSED:
                            itsWidth=WIDTH_CONDENSED;
                            break;
                        case SPD_WIDTH_SEMI_CONDENSED:
                            itsWidth=WIDTH_SEMI_CONDENSED;
                            break;
                        case SPD_WIDTH_NORMAL:
                            itsWidth=WIDTH_NORMAL;
                            break;
                        case SPD_WIDTH_SEMI_EXPANDED:
                            itsWidth=WIDTH_SEMI_EXPANDED;
                            break;
                        case SPD_WIDTH_EXPANDED:
                            itsWidth=WIDTH_EXPANDED;
                            break;
                        default:
                            itsWidth=WIDTH_NORMAL;
                    }
                }

                if(mask&XLFD)
                {
                    itsSpacing=hdr[constSpacingOffset]==constMonospaced ? SPACING_MONOSPACED : SPACING_PROPORTIONAL;
                    hdr[constNoticeOffset+constNoticeNumBytes]='\0';
                    itsFoundry=::getFoundry((const char *)&hdr[constNoticeOffset]);
                }
            }
        }
        spd.close();
    }

    return status;
}

QStringList CFontEngine::getEncodingsSpd()
{
    QStringList enc;

    enc.append("iso8859-1");
    return enc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    BITMAP
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const unsigned int constBitmapMaxProps=1024;

void CFontEngine::createNameBmp(int pointSize, int res, const QString &enc)
{
    QString ptStr,
            resStr;

    ptStr.setNum(pointSize/10);
    resStr.setNum(res);
    itsFullName=itsFamily+" "+weightStr(itsWeight)+(ITALIC_ITALIC==itsItalic ? constBmpItalic : ITALIC_OBLIQUE==itsItalic
                ? constBmpOblique : constBmpRoman)+" ("+ptStr+"pt, "+resStr+"dpi, "+enc+")";
}

static CFontEngine::EItalic charToItalic(char c)
{
    switch(c)
    {
        case 'i':
        case 'I':
            return CFontEngine::ITALIC_ITALIC;
        case 'o':
        case 'O':
            return CFontEngine::ITALIC_OBLIQUE;
        case 'r':
        case 'R':
        default:
            return CFontEngine::ITALIC_NONE;
    }
}

static CFontEngine::ESpacing charToSpacing(char c)
{
    switch(c)
    {
        case 'm':
        case 'M':
            return CFontEngine::SPACING_MONOSPACED;
        case 'c':
        case 'C':
            return CFontEngine::SPACING_CHARCELL;
        default:
        case 'p':
        case 'P':
            return CFontEngine::SPACING_PROPORTIONAL;
    }
}

void CFontEngine::parseXlfdBmp()
{
    enum EXlfd
    {
        XLFD_FOUNDRY=0,
        XLFD_FAMILY,
        XLFD_WEIGHT,
        XLFD_SLANT,
        XLFD_WIDTH,
        XLFD_STYLE,
        XLFD_PIXEL_SIZE,
        XLFD_POINT_SIZE,
        XLFD_RESX,
        XLFD_RESY,
        XLFD_SPACING,
        XLFD_AV_WIDTH,
        XLFD_ENCODING,
        XLFD_END
    };

    int     pos=0,
            oldPos=1,
            pointSize=0,
            res=0;
    QString enc;
    int     entry;

    // XLFD:
    //     -foundry-family-weight-slant-width-?-pixelSize-pointSize-resX-resY-spacing-avWidth-csReg-csEnc

    for(entry=XLFD_FOUNDRY; -1!=(pos=itsXlfd.find('-', pos+1)) && entry<XLFD_END; ++entry)
    {
        switch(entry)
        {
            default:
                break;
            case XLFD_FOUNDRY:
                itsFoundry=itsXlfd.mid(oldPos, pos-oldPos);
                break;
            case XLFD_FAMILY:
                itsFamily=itsXlfd.mid(oldPos, pos-oldPos);
                break;
            case XLFD_WEIGHT:
                itsWeight=strToWeight(itsXlfd.mid(oldPos, pos-oldPos).local8Bit());
                break;
            case XLFD_SLANT:
                if(pos>0)
                    itsItalic=charToItalic(itsXlfd[pos-1].latin1());
                break;
            case XLFD_WIDTH:
                itsWidth=strToWidth(itsXlfd.mid(oldPos, pos-oldPos));
                break;
            case XLFD_STYLE:
                break;
            case XLFD_PIXEL_SIZE:
                itsPixelSize=itsXlfd.mid(oldPos, pos-oldPos).toInt();
                break;
            case XLFD_POINT_SIZE:
                pointSize=itsXlfd.mid(oldPos, pos-oldPos).toInt();
                break;
            case XLFD_RESX:
                res=itsXlfd.mid(oldPos, pos-oldPos).toInt();
                break;
            case XLFD_RESY:
                break;
            case XLFD_SPACING:
                if(pos>0)
                    itsSpacing=charToSpacing(itsXlfd[pos-1].latin1());
                break;
            case XLFD_AV_WIDTH:
                break;
            case XLFD_ENCODING:
                enc=itsXlfd.mid(oldPos);
                break;
        }

        oldPos=pos+1;
    }

    if(XLFD_END==entry)
        createNameBmp(pointSize, res, enc);
    else
        itsFullName=itsXlfd;
}

static const char * getTokenBdf(const char *str, const char *key, bool noquotes=false)
{
    const char   *s=NULL;
    unsigned int keyLen=strlen(key),
                 sLen=strlen(str);

    if(keyLen+1<sLen && NULL!=(s=strstr(str, key)) && (s==str || (!isalnum(s[-1]) && '_'!=s[-1])) &&
      (!noquotes || (noquotes && s[keyLen+1]=='-')))
    {
        const int   constMaxTokenSize=256;
        static char tokenBuffer[constMaxTokenSize];

        char        *end=NULL,
                    *token;

        strncpy(tokenBuffer, s, constMaxTokenSize);
        tokenBuffer[constMaxTokenSize-1]='\0';
        token=tokenBuffer;

        if(noquotes)
        {
            token+=strlen(key)+1;
            if(NULL!=(end=strchr(token, '\n')))
            {
                *end='\0';
                return token;
            }
        }
        else
            if(NULL!=(token=strchr(token, '\"')))
            {
                token++;
                if(NULL!=(end=strchr(token, '\"')))
                {
                    *end='\0';
                    return token;
                }
            }
    }

    return NULL;
}

bool CFontEngine::openFontBdf(const QString &file)
{
    bool            foundXlfd=false;
    CCompressedFile bdf(file);

    if(bdf)
    {
        const int constMaxLineLen=1024;

        char buffer[constMaxLineLen];
        
        while(NULL!=bdf.getString(buffer, constMaxLineLen) && !foundXlfd)
        {
            const char *str;

            if(!foundXlfd && NULL!=(str=getTokenBdf(buffer, "FONT", true)))   // "FONT" does not have quotes!
            {
                if(strlen(str))
                {
                    itsXlfd=str;
                    foundXlfd=true;
                }
                break;
            }
        }

        if(foundXlfd)
            parseXlfdBmp();
    }

    return foundXlfd;
}

static const char * readStrSnf(CCompressedFile &f)
{
    static const int constMaxChars=512;

    static char buffer[constMaxChars];
    int         pos=0;
    char        ch;

    buffer[0]='\0';

    while(-1!=(ch=f.getChar()))
    {
        buffer[pos++]=ch;
        if('\0'==ch)
            break;
    }

    return buffer;
}

bool CFontEngine::openFontSnf(const QString &file)
{
    bool foundXlfd=false;

    struct TCharInfo
    {
        bool         exists()     { return ntohl(misc)&0x80; }
        unsigned int byteOffset() { return (ntohl(misc)&0xFFFFFF00) >> 8; }

        short        leftSideBearing,
                     rightSideBearing,
                     characterWidth,
                     ascent,
                     descent,
                     attributes;
        unsigned int misc;
    };

    struct TGenInfo
    {
        unsigned int version1,
                     allExist,
                     drawDirection,
                     noOverlap,
                     constantMetrics,
                     terminalFont;
        unsigned int misc;
        //unsigned int linear        : 1,
        //             constantWidth : 1,
        //             inkInside     : 1,
        //             inkMetrics    : 1,
        //             padding       : 28;
        unsigned int firstCol,
                     lastCol,
                     firstRow,
                     lastRow,
                     numProps,
                     lenStrings,
                     defaultChar;
        int          fontAscent,
                     fontDescent;
        TCharInfo    minBounds,
                     maxBounds;
        unsigned int pixDepth,
                     glyphSets,
                     version2;
    };

    struct TProp
    {
        unsigned int name,      // string offset of name
                     value,     // Num or string offset
                     indirect;  // true if value is a string
    };

    CCompressedFile snf(file);

    if(snf)
    {
        TGenInfo genInfo;

        if((snf.read(&genInfo, sizeof(TGenInfo))==sizeof(TGenInfo)) && (ntohl(genInfo.version1)==ntohl(genInfo.version2))
           && ntohl(genInfo.numProps)<constBitmapMaxProps)
        {
            TProp        *props=new TProp[ntohl(genInfo.numProps)];
            unsigned int numChars=((ntohl(genInfo.lastCol) - ntohl(genInfo.firstCol)) + 1) * 
                                  ((ntohl(genInfo.lastRow) - ntohl(genInfo.firstRow)) + 1),
                         glyphInfoSize=((genInfo.maxBounds.byteOffset()+3) & ~0x3);

            if(props)
            {
                if(-1!=snf.seek(numChars*sizeof(TCharInfo)+glyphInfoSize, SEEK_CUR))  // Skip character info & glyphs...
                {
                    unsigned int p;
                    bool         error=false;

                    // Now read properties data...
                    for(p=0; p<ntohl(genInfo.numProps); ++p)
                        if(snf.read(&props[p], sizeof(TProp))!=sizeof(TProp))
                        {
                            error=true;
                            break;
                        }
                    if(!error)
                    {
                        const unsigned int constMaxLen=1024;

                        char       buffer[constMaxLen];
                        const char *value=NULL,
                                   *name=NULL;

                        for(p=0; p<ntohl(genInfo.numProps) && !foundXlfd; ++p)
                            if(ntohl(props[p].indirect))
                                if((ntohl(props[p].value)-ntohl(props[p].name))<=constMaxLen &&
                                   -1!=snf.read(buffer, ntohl(props[p].value)-ntohl(props[p].name)))
                                {
                                    name=buffer;
                                    value=readStrSnf(snf);

                                    if(!foundXlfd && CMisc::stricmp(name, "FONT")==0 && strlen(value))
                                    {
                                        foundXlfd=true;
                                        itsXlfd=value;
                                    }
                                }
                                else
                                    break;
                    }
                }
                delete [] props;
            }
        }

        if(foundXlfd)
            parseXlfdBmp();
    }

    return foundXlfd;
}

static unsigned int readLsb32(CCompressedFile &f)
{
    unsigned char num[4];

    if(4==f.read(num, 4))
        return (num[0])+(num[1]<<8)+(num[2]<<16)+(num[3]<<24);
    else
        return 0;
}

static unsigned int read32(CCompressedFile &f, bool msb)
{
    if(msb)
    {
        unsigned char num[4];

        if(4==f.read(num, 4))
            return (num[0]<<24)+(num[1]<<16)+(num[2]<<8)+(num[3]);
        else
            return 0;
    }
    else
        return readLsb32(f);
}

bool CFontEngine::openFontPcf(const QString &file)
{
    bool            foundXlfd=false;
    CCompressedFile pcf(file);

    if(pcf)
    {
        const unsigned int contPcfVersion=(('p'<<24)|('c'<<16)|('f'<<8)|1);

        if(contPcfVersion==readLsb32(pcf))
        {
            const unsigned int constPropertiesType=1;

            unsigned int numTables=readLsb32(pcf),
                         table,
                         type,
                         format,
                         size,
                         offset;

            for(table=0; table<numTables && !pcf.eof() && !foundXlfd; ++table)
            {
                type=readLsb32(pcf);
                format=readLsb32(pcf);
                size=readLsb32(pcf);
                offset=readLsb32(pcf);
                if(constPropertiesType==type)
                {
                    if(pcf.seek(offset, SEEK_SET)!=-1)
                    {
                        const unsigned int constFormatMask=0xffffff00;

                        format=readLsb32(pcf);
                        if(0==(format&constFormatMask))
                        {
                            const unsigned int constByteMask=0x4;

                            bool         msb=format&constByteMask;
                            unsigned int numProps=read32(pcf, msb);

                            if(numProps>0 && numProps<constBitmapMaxProps)
                            {
                                unsigned int strSize,
                                             skip;

                                struct TProp
                                {
                                    unsigned int name,
                                                 value;
                                    bool         isString;
                                } *props=new struct TProp [numProps];

                                if(props)
                                {
                                    char           tmp;
                                    unsigned short prop;

                                    for(prop=0; prop<numProps; ++prop)
                                    {
                                        props[prop].name=read32(pcf, msb);
                                        pcf.read(&tmp, 1);
                                        props[prop].isString=tmp ? true : false;
                                        props[prop].value=read32(pcf, msb);
                                    }

                                    skip=4-((numProps*9)%4);
                                    if(skip!=4)
                                        pcf.seek(skip, SEEK_CUR);

                                    strSize=read32(pcf, msb);

                                    if(strSize>0)
                                    {
                                        char *str=new char [strSize];
                                        if(pcf.read(str, strSize)==(int)strSize)
                                        {
                                            // Finally we have the data............
                                            const int constMaxStrLen=1024;

                                            char tmp[constMaxStrLen];

                                            for(prop=0; prop<numProps && !foundXlfd; ++prop)
                                                if(CMisc::stricmp(&str[props[prop].name], "FONT")==0)
                                                {
                                                    if(props[prop].isString && strlen(&str[props[prop].value]))
                                                    {
                                                        foundXlfd=true;
                                                        strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                        tmp[constMaxStrLen-1]='\0';
                                                        itsXlfd=tmp;
                                                    }
                                                    break;
                                                }
                                        }
                                        delete [] str;
                                    }
                                    delete [] props;
                                }
                            }
                        }
                    }
                    break;   // Forget the other tables...
                }
            }
        }

        if(foundXlfd)
            parseXlfdBmp();
    }
    return foundXlfd;
}

#ifdef HAVE_FT_CACHE
FTC_FaceID CFontEngine::getId(const QString &f, int faceNo)
{
    TId *p=NULL;

    for(p=itsFt.ids.first(); p; p=itsFt.ids.next())
        if (p->path==f && p->faceNo==faceNo)
            break;

    if(!p)
    {
        p=new TId(f, faceNo);
        itsFt.ids.append(p);
    }

    return (FTC_FaceID)p;
}

#if KFI_FT_IS_GE(2, 1, 8)
bool CFontEngine::getGlyphBitmap(FTC_ImageTypeRec &font, FT_ULong index, Bitmap &target, int &left, int &top,
                                 int &xAdvance, FT_Pointer *ptr)
#else
bool CFontEngine::getGlyphBitmap(FTC_Image_Desc &font, FT_ULong index, Bitmap &target, int &left, int &top,
                                 int &xAdvance, FT_Pointer *ptr)
#endif
{
    bool ok=false;

    *ptr=NULL;

    //
    // Cache small glyphs, else render on demand...
#if KFI_FT_IS_GE(2, 1, 8)
    if(font.width<48 && font.height<48)
#else
    if(font.font.pix_width<48 && font.font.pix_height<48)
#endif
    {
        FTC_SBit sbit;

#if KFI_FT_IS_GE(2, 1, 8)
        if(!FTC_SBitCache_Lookup(itsFt.sBitCache, &font, index, &sbit, NULL) && sbit->buffer)
#else
        if(!FTC_SBit_Cache_Lookup(itsFt.sBitCache, &font, index, &sbit) && sbit->buffer)
#endif
        {
            target.greys=sbit->max_grays+1; // ft_pixel_mode_mono==sbit->format ? 2 : 256;
            target.mono=ft_pixel_mode_mono==sbit->format ? true : false;
            target.pitch=sbit->pitch;
            target.height=sbit->height;
            target.width=sbit->width;
            target.buffer=sbit->buffer;
            left=sbit->left;
            top=sbit->top;
            xAdvance=sbit->xadvance;

            ok=true;
        }
    }
    if(!ok)
    {
        FT_Glyph glyph;

#if KFI_FT_IS_GE(2, 1, 8)
        if(!FTC_ImageCache_Lookup(itsFt.imageCache, &font, index, &glyph, NULL))
#else
        if(!FTC_Image_Cache_Lookup(itsFt.imageCache, &font, index, &glyph))
#endif
        {
            ok=true;

            if(ft_glyph_format_outline==glyph->format)
                if(ok=!FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 0))
                    *ptr=glyph;

            if(ok)
                if(ft_glyph_format_bitmap==glyph->format && ((FT_BitmapGlyph)glyph)->bitmap.buffer)
                {
                    FT_BitmapGlyph bitmap=(FT_BitmapGlyph)glyph;
                    FT_Bitmap      *source=&(bitmap->bitmap);

                    target.greys=ft_pixel_mode_mono==(FT_Pixel_Mode_) source->pixel_mode ? 2 : source->num_grays;
                    target.mono=ft_pixel_mode_mono==(FT_Pixel_Mode_) source->pixel_mode ? true : false;
                    target.pitch=source->pitch;
                    target.height=source->rows;
                    target.width=source->width;
                    target.buffer=source->buffer;
                    left=bitmap->left;
                    top=bitmap->top;
                    xAdvance=(glyph->advance.x+0x8000)>>16;
                }
                else
                    ok=false;
        }
    }

    return ok;
}

void CFontEngine::align32(Bitmap &bmp)
{
    // Pitch = number of bytes per row of the bitmap. This needs to fall on a 32bit (4byte) boundary.
    int padBytes=4-(bmp.pitch%4);

    if(padBytes<4)
    {
        int size=(bmp.pitch+padBytes)*bmp.height,
            row;

        if(size>itsFt.bufferSize)
        {
            static const int constBufferBlock=512;

            if(itsFt.buffer)
                delete [] itsFt.buffer;
            itsFt.bufferSize=(((int)(size/constBufferBlock))+(size%constBufferBlock ? 1 : 0))*constBufferBlock;
            itsFt.buffer=new unsigned char [itsFt.bufferSize];
        }

        //memset(itsFt.buffer, 0, itsFt.bufferSize);
        for(row=0; row<bmp.height; ++row)
            memcpy(&(itsFt.buffer[row*(bmp.pitch+padBytes)]), &bmp.buffer[row*bmp.pitch], bmp.pitch);

        bmp.buffer=itsFt.buffer;
    }
}

#if KFI_FT_IS_GE(2, 1, 8)
bool CFontEngine::drawGlyph(QPixmap &pix, FTC_ImageTypeRec &font, int glyphNum,
                            FT_F26Dot6 &x, FT_F26Dot6 &y, FT_F26Dot6 width, FT_F26Dot6 height,
                            FT_F26Dot6 startX, FT_F26Dot6 stepY, bool multiLine)
#else
bool CFontEngine::drawGlyph(QPixmap &pix, FTC_Image_Desc &font, int glyphNum,
                            FT_F26Dot6 &x, FT_F26Dot6 &y, FT_F26Dot6 width, FT_F26Dot6 height,
                            FT_F26Dot6 startX, FT_F26Dot6 stepY, bool multiLine)
#endif
{
    int        left,
               top,
               xAdvance;
    FT_Pointer glyph;
    Bitmap     bmp;

    if(getGlyphBitmap(font, glyphNum, bmp, left, top, xAdvance, &glyph) && bmp.width>0 && bmp.height>0)
    {
        if(x+xAdvance+1>width)
        {
            if(!multiLine)
                return true;

            x=startX;
            y+=stepY;

            if(y>height)
                return true;
        }

        static QRgb clut8[256];
        static QRgb clut1[2]={ qRgb(255, 255, 255), qRgb(0, 0, 0) };
        static bool clut8Setup=false;

        if(!bmp.mono && !clut8Setup)
        {
            int j;
            for(j=0; j<256; j++)
                clut8[j]=qRgb(255-j, 255-j, 255-j);
            clut8Setup=true;
        }

        align32(bmp);

        QPixmap glyphPix(QImage(bmp.buffer, bmp.width, bmp.height, bmp.mono ? 1 : 8, bmp.mono ? clut1 : clut8,
                                bmp.mono ? 2 : bmp.greys, QImage::BigEndian));

        bitBlt(&pix, x+left, y-top, &glyphPix, 0, 0, glyphPix.width(), glyphPix.height(), Qt::AndROP);

        if(glyph)
            FT_Done_Glyph((FT_Glyph)glyph);

        x+=xAdvance+1;
    }
    else if(x!=startX)
#if KFI_FT_IS_GE(2, 1, 8)
        x+=(font.width/3);
#else
        x+=(font.font.pix_width/3);
#endif
    return false;
}
#endif

CFontEngine::TFtData::TFtData()
                    : open(false)
#ifdef HAVE_FT_CACHE
                    , buffer(NULL),
                      bufferSize(0)
#endif
{
    if(FT_Init_FreeType(&library))
    {
        std::cerr << "ERROR: FreeType2 failed to initialise\n";
        exit(0);
    }
#ifdef HAVE_FT_CACHE
    ids.setAutoDelete(true);
    if(FTC_Manager_New(library, 0, 0, 0, face_requester, 0, &cacheManager))
    {
        std::cerr << "ERROR: Could not initliaze FreeType2 cache manager\n";
        exit(0);
    }
#if KFI_FT_IS_GE(2, 1, 8)
    if(FTC_SBitCache_New(cacheManager, &sBitCache))
#else
    if(FTC_SBit_Cache_New(cacheManager, &sBitCache))
#endif
    {
        std::cerr << "ERROR: Could not initliaze FreeType2 small bitmaps cache\n";
        exit(0);
    }
#if KFI_FT_IS_GE(2, 1, 8)
    if(FTC_ImageCache_New(cacheManager, &imageCache))
#else
    if(FTC_Image_Cache_New(cacheManager, &imageCache))
#endif
    {
        std::cerr << "ERROR: Could not initliaze FreeType2 glyph image cache\n";
        exit(0);
    }
#endif
}

CFontEngine::TFtData::~TFtData()
{
#ifdef HAVE_FT_CACHE
    FTC_Manager_Done(cacheManager);
#endif
    FT_Done_FreeType(library);
}
