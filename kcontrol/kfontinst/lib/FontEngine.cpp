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

static const char *constNoPsName  ="NO_PS_NAME";
static const char *constBmpRoman  ="";
static const char *constBmpItalic =" Italic";
static const char *constBmpOblique=" Oblique";
static const char *constOblique   ="Oblique";
static const char *constSlanted   ="Slanted";

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
    closeFont();

    itsType=getType(QFile::encodeName(file));
    itsWeight=WEIGHT_MEDIUM;
    itsWidth=WIDTH_NORMAL;
    itsSpacing=SPACING_PROPORTIONAL;
    itsItalicAngle=0;
    itsItalic=ITALIC_NONE;
    itsEncoding=itsAfmEncoding=QString::null;
    itsFt.open=false;
    itsNumFaces=1;
    itsPixelSize=0;
    itsPath=file;

    switch(itsType)
    {
        case TRUE_TYPE:
        case TT_COLLECTION:
        case OPEN_TYPE:
            return openFontTT(file, mask, face);
        case TYPE_1:
            return openFontT1(file, mask);
        case SPEEDO:
            return openFontSpd(file, mask);
        case BITMAP:
            return openFontBmp(file);
        case TYPE_1_AFM:
            return openFontAfm(file);
        default:
            if(force)
            {
                bool ok=openFontT1(file, mask);

                if(ok)
                    itsType=TYPE_1;
                else
                {
                    ok=openFontTT(file, mask, face);
                    if(ok)
                        itsType=itsNumFaces>1 ? TT_COLLECTION : TRUE_TYPE;
                    else
                    {
                        ok=openFontSpd(file, mask);
                        if(ok)
                            itsType=SPEEDO;
                        else
                        {
                            ok=openFontBmp(file, force);
                            if(ok)
                                itsType=BITMAP;
                            else
                            {
                                ok=openFontAfm(file);
                                if(ok)
                                    itsType=TYPE_1_AFM;
                            }
                        }
                    }
                }

                return ok;
            }
            else
                return false;
    }
}

bool CFontEngine::openKioFont(const QString &file, unsigned short mask, bool force, int face)
{
    if(openFont(file, mask, force, face))
        return true;
    else
    {
        const QStringList          &list=CGlobal::cfg().getRealTopDirs(file);
        QStringList::ConstIterator it;

        for(it=list.begin(); it!=list.end(); it++)
        {
            QString fname(*it+CMisc::getSub(file));

            if(CMisc::fExists(fname) && openFont(fname, mask, force, face))
            {
                itsPath=*it+CMisc::getSub(file);
                return true;
            }
        }
    }
    return false; 
}

void CFontEngine::closeFont()
{
    if(itsFt.open)
    {
        FT_Done_Face(itsFt.face);
        itsFt.open=false;
    }

    itsType=NONE;
}

bool CFontEngine::isA(const char *fname, const char *ext, bool z)
{
    int  len=strlen(fname);
    bool fnt=false;

    if(z)
    {
        if(len>7)                 // Check for .ext.gz
            fnt=(fname[len-7]=='.' && tolower(fname[len-6])==ext[0] && tolower(fname[len-5])==ext[1] && tolower(fname[len-4])==ext[2] &&
                 fname[len-3]=='.' && tolower(fname[len-2])=='g' && tolower(fname[len-1])=='z');

        if(!fnt && len>6)         // Check for .ext.Z
            fnt=(fname[len-6]=='.' && tolower(fname[len-5])==ext[0] && tolower(fname[len-4])==ext[1] && tolower(fname[len-3])==ext[2] &&
                 fname[len-2]=='.' && toupper(fname[len-1])=='Z');
    }

    if(!fnt && len>4)  // Check for .ext
        fnt=(fname[len-4]=='.' && tolower(fname[len-3])==ext[0] && tolower(fname[len-2])==ext[1] && tolower(fname[len-1])==ext[2]);

    return fnt;
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
    if(isASpeedo(fname))
        return SPEEDO;
    if(isABitmap(fname))
        return BITMAP;
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

QStringList CFontEngine::getEncodings()
{
    switch(itsType)
    {
        case TRUE_TYPE:
        case TT_COLLECTION:
        case OPEN_TYPE:
            return getEncodingsFt();
        case TYPE_1:
            return getEncodingsT1();
        case SPEEDO:
            return getEncodingsSpd();
        case BITMAP:
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
        return WEIGHT_UNKNOWN;
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
        return WEIGHT_REGULAR;
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
        return WEIGHT_UNKNOWN;
}

CFontEngine::EWidth CFontEngine::strToWidth(const QString &str)
{
    if(str.isEmpty())
        return WIDTH_UNKNOWN;
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
    QString name;
    int     face=0,
            numFaces=0;

    do
    {
        if(openKioFont(file, NAME, force, face))
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

        for(ch=0; ch<str.length() && found; ch++)
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

void CFontEngine::createPreview(int width, int height, QPixmap &pix, int faceNo)
{
    FT_Face        face;
    FT_Size        size;
    FTC_Image_Desc font;
    bool           thumb=width==height && height<=128;
    int            fontSize=28,
                   offset=4,
                   space=8;

    if(thumb)
    {
        if(height<=32)
        {
            offset=2;
            space=0;
            fontSize=(height-(offset*2))-2;
        }
        else
        {
            offset=3;
            space=3;
            fontSize=((height-(offset*3))-6)/2;
        }
    }

    font.font.face_id=getId(itsPath, faceNo);
    font.font.pix_width=font.font.pix_height=BITMAP==itsType ? itsPixelSize : thumb ? fontSize : point2Pixel(fontSize);
    font.image_type=ftc_image_grays;

    FT_F26Dot6 startX=offset,
               startY=offset+font.font.pix_height,
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

        if(BITMAP==itsType)
        {
            int pos=name.findRev('(');

            info=name.mid(pos);
            name=name.left(pos);
        }

        title.setPixelSize(12);
        painter.setFont(title);
        painter.setPen(Qt::black);
        y=painter.fontMetrics().height();
        drawText(painter, x, y, width, name);

        if(BITMAP==itsType)
        {
            y+=2+painter.fontMetrics().height();
            drawText(painter, x, y, width, info);
        }

        y+=4;
        painter.drawLine(offset, y, width-offset, y);
        y+=2;
        y+=startY;
    }

    if(!FTC_Manager_Lookup_Size(itsFt.cacheManager, &(font.font), &face, &size))
    {
        int        i;
        FT_F26Dot6 stepY=size->metrics.y_ppem+offset;

        if(!thumb)
        {
            QString  quote(i18n("A sentence that uses all of the letters of the alphabet", "The quick brown fox jumps over the lazy dog"));
            bool     foundCmap=getCharMap(face, quote);

            if(foundCmap)
            {
                unsigned int ch;

                for(ch=0; ch<quote.length(); ++ch)
                    if(drawGlyph(pix, font, FT_Get_Char_Index(face, quote[ch].unicode()),
                       x, y, width, height, startX, stepY, space))
                        break;
            }

            if(BITMAP!=itsType)
                font.font.pix_width=font.font.pix_height=point2Pixel((int)(fontSize*0.75));

            if(y<height && !FTC_Manager_Lookup_Size(itsFt.cacheManager, &(font.font), &face, &size))
            {
                FT_F26Dot6 stepY=size->metrics.y_ppem+offset;

                if(foundCmap)
                {
                    if(x!=startX)
                    {
                        y+=stepY;
                        x=startX;
                    }

                    y+=font.font.pix_height;
                }

                for(i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
                    if(drawGlyph(pix, font, i, x, y, width, height, startX, stepY))
                        break;
            }
        }
        else
        {
            QString str(i18n("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));

            if(getCharMap(face, str))
            {
                unsigned int ch;

                for(ch=0; ch<str.length(); ++ch)
                    if(drawGlyph(pix, font, FT_Get_Char_Index(face, str[ch].unicode()),
                       x, y, width, height, startX, stepY))
                        break;

            }
            else
                for(i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
                    if(drawGlyph(pix, font, i, x, y, width, height, startX, stepY))
                        break;
        }

    }
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TrueType, Type1, and Speedo
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool lookupName(FT_Face face, int nid, int pid, int eid, FT_SfntName *nameReturn)
{
    int n = FT_Get_Sfnt_Name_Count(face);

    if(n>0)
    {
        int         i;
        FT_SfntName name;

        for(i=0; i<n; i++)
            if(0==FT_Get_Sfnt_Name(face, i, &name) && name.name_id == nid && name.platform_id == pid && (eid < 0 || name.encoding_id == eid))
            {
                switch(name.platform_id)
                {
                    case TT_PLATFORM_APPLE_UNICODE:
                    case TT_PLATFORM_MACINTOSH:
                        if(name.language_id != TT_MAC_LANGID_ENGLISH)
                            continue;
                        break;
                    case TT_PLATFORM_MICROSOFT:
                        if(name.language_id != TT_MS_LANGID_ENGLISH_UNITED_STATES && name.language_id != TT_MS_LANGID_ENGLISH_UNITED_KINGDOM)
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

    if(lookupName(face, nid, TT_PLATFORM_MICROSOFT, TT_MS_ID_UNICODE_CS, &name) || lookupName(face, nid, TT_PLATFORM_APPLE_UNICODE, -1, &name))
        for(unsigned int i=0; i < name.string_len / 2; i++)
            str+=0 == name.string[2*i] ? name.string[(2*i)+1] : '_';
    else if(lookupName(face, nid, TT_PLATFORM_MACINTOSH, TT_MAC_ID_ROMAN, &name)) // Pretend that Apple Roman is ISO 8859-1.
        for(unsigned int i=0; i < name.string_len; i++)
            str+=name.string[i];

    return str;
}

static const char * const constDefaultFoundry = "Misc";

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

static const char * getFoundry(const FT_Face face)
{
    struct Map
    {
        const char *vendorId,
                   *foundry;
    };

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
        { "LARA", "LarabieFonts"},
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
        { "ABC" , "AltekInst"},
        { "ACUT", "AcuteType"},
        { "AOP" , "AnArtofP"},
        { "AZLS", "AzaleaInc"},
        { "BARS", "CIA UK"},
        { "BERT", "Berthold"},
        { "BITM", "Bitmap Soft"},
        { "BLAH", "MisterBlas"},
        { "BOYB", "I.Frances"},
        { "BRTC", "BearRockTech"},
        { "BWFW", "B/WFontworks"},
        { "C&C",  "Carter&Cone"},
        { "CAK" , "Pluginfonts"},
        { "CASL", "H.W.Caslon"},
        { "COOL", "CoolFonts"},
        { "CTDL", "ChinaTyDesLtd"},
        { "DAMA", "D.M.Ltd"},
        { "DS"  , "DSMCoInc"},
        { "DSCI", "DesScInc"},
        { "DTC",  "DTCorp"},
        { "DTPS", "DTPS"},
        { "DUXB", "DSysInc"},
        { "ECF" , "E.C.F."},
        { "EDGE", "R.E.Corp."},
        { "EFF" , "EFF"},
        { "EFNT", "EFLLC"},
        { "ELSE", "Elseware"},
        { "ERAM", "Eraman"},
        { "ESIG", "E-Signature"},
        { "FBI",  "TFBI"},
        { "FCAB", "TFC"},
        { "FONT", "FontSource"},
        { "FS"  , "FormulaSls"},
        { "FSE" , "FontSrc"},
        { "FSI" , "FSIGmbH"},
        { "FTFT", "FontFont"},
        { "FWRE", "FontwareLtd"},
        { "GALA", "Galapagos"},
        { "GD"  , "GDFonts"},
        { "GLYF", "GlyphSys"},
        { "GPI",  "GammaProd"},
        { "HY",   "HanYangSys"},
        { "HILL", "Hill Sys"},
        { "HOUS", "HouseInd"},
        { "HP",   "HP"},
        { "IDEE", "IDEE"},
        { "IDF",  "IntDigital"},
        { "ILP",  "IndLang"},
        { "ITF" , "IntTypeInc"},
        { "KATF", "Kingsley"},
        { "LANS", "Lanston"},
        { "LGX" , "LogixRII"},
        { "LING", "Linguists"},
        { "LP",   "LetterPer"},
        { "LTRX", "Lighttracks"},
        { "MC"  , "Cerajewski"},
        { "MILL", "Millan"},
        { "MJ"  , "MajusCorp"},
        { "MLGC", "Micrologic"},
        { "MSCR", "MajusCorp"},
        { "MTY" , "MotoyaCoLTD"},
        { "MUTF", "CACHE"},
        { "NB"  , "NoBodoniTyp"},
        { "NDTC", "NeufvilleDig"},
        { "NIS" , "NIS Corp"},
        { "ORBI", "OrbitEntInc"},
        { "P22" , "P22 Inc"},
        { "PDWX", "ParsonsDes"},
        { "PF"  , "PhilsFonts"},
        { "PRFS", "Production"},
        { "RKFN", "R K Fonts"},
        { "robo", "Buro Petr"},
        { "RUDY", "RudynFluffy"},
        { "SAX" , "SAX gmbh"},
        { "Sean", "The FontSite"},
        { "SFI" , "SoftwareF"},
        { "SFUN", "Soft Union"},
        { "SG"  , "ScooterGfx"},
        { "SIG" , "Signature"},
        { "SKZ" , "CelticLadys"},
        { "SN"  , "SourceNet"},
        { "SOHO", "SoftHorizons"},
        { "SOS" , "StandingOvs"},
        { "STF" , "BrianSooy"},
        { "STON", "ZHUHAI"},
        { "SUNW", "Sunwalk"},
        { "SWFT", "SwfteInt"},
        { "SYN" , "SynFonts"},
        { "TDR" , "TansinADarcs"},
        { "TF"  , "Treacyfaces"},
        { "TILD", "SIATilde"},
        { "TPTC", "TESTPILOT"},
        { "TR"  , "TypeRevivals"},
        { "TS"  , "TamilSoft"},
        { "UA"  , "UnAuthorized"},
        { "VKP" , "VijayKPatel"},
        { "VLKF", "Visualogik"},
        { "VOG" , "MartinVogel"},
        { "ZEGR", "ZebraFontFacit"},
        { "ZETA", "TangramStudio"},
        { "ZSFT", "ZSoft"},
        { NULL  ,  NULL}
    };

    void *table=FT_Get_Sfnt_Table(face, ft_sfnt_os2);

    if(NULL!=table && 0xFFFF!=((TT_OS2*)table)->version)
    {
        const char *vendor=(const char *)((TT_OS2*)table)->achVendID;
        const Map  *entry;

        for(entry=map; NULL!=entry->vendorId; entry++)
        {
            unsigned int len=strlen(entry->vendorId);

            if(0==memcmp(entry->vendorId, vendor, len))
            {
                bool ok=true;

                for(int i=len; i<4 && ok; i++)
                    if(vendor[i]!=' ' && entry->vendorId[i]!='\0')
                        ok=false;

                if(ok)
                    return entry->foundry;
            }
        }
    }
                
    const char *foundry=NULL;

#if ((FREETYPE_MAJOR > 2) || ((FREETYPE_MAJOR == 2) && (FREETYPE_MINOR >= 1)))
    PS_FontInfoRec t1info;

    if(0==FT_Get_PS_Font_Info(face, &t1info))
        foundry=getFoundry(t1info.notice, true);
#endif

    if(!foundry)
        foundry=getFoundry(getName(face, TT_NAME_ID_TRADEMARK));

    if(!foundry)
        foundry=getFoundry(getName(face, TT_NAME_ID_MANUFACTURER));

    return foundry ? foundry : constDefaultFoundry;
}

static void removeString(QString &str, const QString &remove, QCString &removed, bool store=true)
{
    static const QChar space(' '),
                       dash('-');
    int                pos;
    unsigned int       slen=remove.length();

    if(0<(pos=str.find(remove, 0, false)) && (space==str[pos-1] || dash==str[pos-1]) &&
       (str.length()<=pos+slen || space==str[pos+slen] || dash==str[pos+slen]))
    {
        str.remove(pos-1, slen+1);

        if(store)
        {
            removed+=remove.latin1();
            removed+=" ";
        }
    }
}

static QString createNames(const QString &familyName, QString &fullName)
{
    //
    // Some fonts have a FamilyName like "Wibble Wobble"
    // and a FullName like               "Wibble Wobble Bold Italic Oldstyle"
    // ...Therefore can't just use the family name read in from the font - as this would ignore the "Oldstyle" part.
    // So, use the "FullName", and remove the any style information (weight, width, italic/roman/oblique) from this...
    //
    // NOTE: Can't simply use FullName and remove style info - as this would convert "Times New Roman" to "Times New"!!
    //

    QString  family(fullName);
    QCString removed;
    bool     removedFamily=true;

    //
    // Remove family name...
    if(!familyName.isEmpty())
        if(0==family.find(familyName))    // This removes "Times New Roman" from "Times New Roman Bold" -- this is the gneral case...
            family.remove(0, familyName.length());
        else
        {
            //
            // Some fonts have the family name listed with no spaces, but have spaces in the full name, e.g.
            //
            //     Family   : LuciduxMono
            //     FullName : Lucidux Mono Italic Oldstyle
            //
            // ...therefore, need to remove each string from FullName that occurs in FamilyName
            QString full(fullName),
                    fam(familyName);

            full.replace(" ", QString::null);   // Remove whitespace - so we would now have "LuciduxMonoItalicOldstyle"
            fam.replace(" ", QString::null);

            if(0==full.find(fam))  // Found "LuciduxMono" in "LuciduxMonoItalic" - so set family to "ItalicOldstyle"
            {
                //
                // Now we need to extract the family name, and the rest...
                // i.e. Family: "LuciduxMono"
                //      rest  : "Italic Oldstyle" -- this is what get's assigned to the 'family' varaible...

                if(full.length()==fam.length())  // No style information...
                    family="";
                else  // Need to remove style info...
                {
                    unsigned int i;

                    //
                    // Remove familyName from family
                    for(i=0; i<familyName.length() && family.length(); ++i)
                    {
                        if(family[0]==QChar(' '))
                            family.remove(0, 1);
                        if(family.length())
                            family.remove(0, 1);
                    }
                }
            }
            else
            {
                //
                // FamilyName and family name within FullName are different...
                removedFamily=false;
            }
        }

    //
    // Remove widthm, weight, and italic stuff from fullName...
    int prop;

    for(prop=CFontEngine::WEIGHT_THIN; prop<=CFontEngine::WEIGHT_BLACK; prop++)
        removeString(family, CFontEngine::weightStr((CFontEngine::EWeight)prop), removed);

    removeString(family, "Italic", removed);
    removeString(family, constOblique, removed);
    removeString(family, constSlanted, removed);

    //
    // Most fonts don't list the roman part, if some do then we don't really
    // want to show this - to make everything as similar as possible...
    removeString(family, "Roman", removed, false);

    for(prop=CFontEngine::WIDTH_ULTRA_CONDENSED; prop<=CFontEngine::WIDTH_ULTRA_EXPANDED; prop++)
        removeString(family, CFontEngine::widthStr((CFontEngine::EWidth)prop), removed);

    removeString(family, "Cond", removed);  // Some fonts just have Cond and not Condensed!

    //
    // Remvoe any "Plain:1.0", etc, strings...
    int plPos;

    if(-1!=(plPos=family.find(" Plain:")))
    {
        int spPos=family.find(QChar(' '), plPos+1);
        family.remove(plPos, -1==spPos ? family.length()-plPos : spPos-plPos);
    }

    //
    // Add the family name back on...
    if(removedFamily && !familyName.isEmpty())
        family=familyName+family;

    //
    // Replace any non-alphanumeric or space characters...
    family.replace(QRegExp("&"), "And");
    family=CMisc::removeSymbols(family);
    family=family.simplifyWhiteSpace();
    family=family.stripWhiteSpace();

    if(!removed.isEmpty())
    {
        QCString fn(removedFamily ? family.latin1() : familyName.latin1());

        fn+=" ";
        fn+=removed;
        fullName=fn.stripWhiteSpace();
    }
    else
        fullName=removedFamily ? family : familyName;

    return removedFamily ? family : familyName;
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
const char * CFontEngine::getReadOnlyTokenT1(const char *str, const char *key)
{
    //
    // Read only tokens have the format:
    //
    //   <key> ( <token> ) readonly def
    // e.g. /FullName (Utopia Bold Italic) readonly def
    //
    static const int constMaxTokenLen=1024;

    static char token[constMaxTokenLen];

    char *start,
         *end;

    token[0]='\0';
    if(NULL!=(start=strstr(str, key)) && NULL!=(start=strchr(start, '(')) && NULL!=(end=strstr(start, "readonly")))
    {
        start+=1;  // Move past open bracket
        // go backwards from 'readonly', until we get to ')'

        for(; end>start; end--)
            if(*end==')')  // Then we've found it...
            {
                unsigned int numChars=end-start;

                if(numChars>constMaxTokenLen-1)
                    numChars=constMaxTokenLen-1;
                strncpy(token, start, numChars);
                token[numChars]='\0';
                break;
            }
    }

    return token[0]=='\0' ? NULL : token;
}

const char * CFontEngine::getTokenT1(const char *str, const char *key)
{
    //
    // Tokens have the format:
    //
    //   <key> <token> def
    // e.g. /isFixedPitch false def
    //
    static const int constMaxTokenLen=1024;

    static char token[constMaxTokenLen];

    char *start,
         *end;

    token[0]='\0';
    if(NULL!=(start=strstr(str, key)) && NULL!=(end=strstr(start, "def")) && end>start)
    {
        for(start+=strlen(key); *start==' ' || *start=='\t'; ++start)  // Skip past any spaces...
            ;

        for(end--; *end==' ' || *end=='\t'; --end)   //  Ditto
            ;
        unsigned int numChars=(end-start)+1;

        if(numChars>constMaxTokenLen-1)
            numChars=constMaxTokenLen-1;
        strncpy(token, start, numChars);
        token[numChars]='\0';
    }

    return token[0]=='\0' ? NULL : token;
}

bool CFontEngine::openFontT1(const QString &file, unsigned short mask)
{
    const int constHeaderMaxLen=4096;  // Read in the first X bytes, this should be enough for just the header data.

    char data[constHeaderMaxLen];
    bool status=false;

    if(TEST==mask || mask&XLFD)
    {
        if(FT_New_Face(itsFt.library, QFile::encodeName(file), 0, &itsFt.face))
            return false;
        else
            itsFt.open=true;
    }

    if(TEST==mask)
        status=true; // If we've reached this point, then the FT_New_Face worked!
    else
    {
        std::ifstream f(QFile::encodeName(file));

        if(f)
        {
            unsigned char *hdr=(unsigned char *)data;

            f.read(data, constHeaderMaxLen);
            int bytesRead=f.tellg();
            f.close();

            data[bytesRead-1]='\0';

            bool binary=(hdr[0]==0x80 && hdr[1]==0x01) || (hdr[0]==0x01 && hdr[1]==0x80);

            if(bytesRead>2 && ( binary || (strstr(data, "%!")==data) ) )
            {
                const char *header,
                           *str,
                           *dict;
                char       *end;
                bool       foundName=false,
                           foundFamily=false,
                           foundPs=false,
                           foundNotice=false,
                           foundEncoding=false,
                           familyIsFull=false;

                header=binary ? &data[6] : data;

                // Look for start of 'dict' section...
                if((dict=strstr(header, "dict begin"))!=NULL)
                {
                    // Now look for the end of the 'dict' section
                    if((end=strstr(dict, "currentdict end"))!=NULL)
                        *end='\0';  // If found, then set to NULL - this should speed up the following strstr's

                    // Having found the 'dict' section, now try to read the data...

                    if(NULL!=(str=getTokenT1(dict, "/Encoding")))
                    {
                        itsEncoding=str;
                        foundEncoding=true;
                    }

                    if((mask&NAME || mask&XLFD || mask&PROPERTIES) && !foundName && NULL!=(str=getReadOnlyTokenT1(dict, "/FullName")))
                    {
                        itsFullName=str;
                        foundName=true;
                    }

                    if((mask&NAME || mask&XLFD || mask&PROPERTIES) && NULL!=(str=getTokenT1(dict, "/FontName")))
                    {
                        itsPsName=str[0]=='/' ? &str[1] : str;
                        foundPs=true;
                    }

                    if(mask&NAME || mask&PROPERTIES || mask&XLFD)
                    {
                        if(NULL!=(str=getReadOnlyTokenT1(dict, "/FamilyName")))
                        {
                            itsFamily=str;
                            foundFamily=true;
                        }
                        if(NULL!=(str=getReadOnlyTokenT1(dict, "/Weight")))
                            itsWeight=strToWeight(str);
                        if(NULL!=(str=getTokenT1(dict, "/ItalicAngle")))
                        {
                            itsItalicAngle=atof(str);
                            itsItalic=itsItalicAngle== 0.0f ? ITALIC_NONE : ITALIC_ITALIC;
                        }
                    }

                    if(mask&XLFD)
                    {
                        if(NULL!=(str=getTokenT1(dict, "/isFixedPitch")))
                            itsSpacing=strstr(str, "false")==str ? SPACING_PROPORTIONAL : SPACING_MONOSPACED;
                        if(NULL!=(str=getReadOnlyTokenT1(dict, "/Notice")))
                        {
                            itsFoundry=::getFoundry(str);
                            foundNotice=true;
                        }
                    }

                    if(mask&XLFD && !foundNotice)
                    {
                        foundNotice=true;
                        itsFoundry=constDefaultFoundry;
                    }
                }

                if(mask&NAME || mask&PROPERTIES || mask&XLFD)
                    if(!foundName && foundPs)  // The Hershey fonts don't have the FullName or FamilyName parameters!...
                    {
                        itsFullName=itsPsName;
                        itsFullName.replace(QRegExp("\\-"), " ");
                        foundName=true;
                    }

                if(mask&PROPERTIES || mask&XLFD)
                    if(!foundFamily && foundName)
                    {
                        itsFamily=itsFullName;
                        familyIsFull=true;
                        foundFamily=true;
                    }

                if((mask&XLFD || mask&NAME) && foundName)
                    itsWidth=strToWidth(itsFullName);

                if(mask&XLFD && !foundNotice)
                {
                    foundNotice=true;
                    itsFoundry=constDefaultFoundry;
                }

                if(foundName && (mask&PROPERTIES || mask&XLFD || mask&NAME))
                    itsItalic=checkItalic(itsItalic, itsFullName);

                if(foundName && foundFamily)
                    itsFamily=createNames(familyIsFull ? QString::null : itsFamily, itsFullName);

                status= ( (mask&NAME && !foundName) || (mask&PROPERTIES && (!foundPs || !foundFamily)) ||
                        (mask&XLFD && (!foundNotice || !foundName || !foundEncoding)) ) ? false : true;
            }
        }
    }

    if(status && mask&XLFD && getIsArrayEncodingT1()) // Read encoding from .afm, if it exists...
    {
        QString afm(CMisc::afmName(file));

        if(CMisc::fExists(afm))
        {
            std::ifstream f(QFile::encodeName(afm));
 
            if(f)
            {
                const int  constMaxLen=512;
                const char *contEncStr="EncodingScheme";
 
                char line[constMaxLen],
                     enc[constMaxLen],
                     *pos;
 
                do
                {
                    f.getline(line, constMaxLen);
 
                    if(f.good())
                    {
                        line[constMaxLen-1]='\0';
                        if(NULL!=(pos=strstr(line, contEncStr)) && strlen(pos)>(strlen(contEncStr)+1) && 1==sscanf(&(pos[strlen(contEncStr)]), "%s", enc))
                        {
                            itsAfmEncoding=enc;
                            break;
                        }
                   }
                }
                while(!f.eof());
                f.close();
            }
        }
    }

    return status;
}

QStringList CFontEngine::getEncodingsT1()
{
    QStringList enc;

    if(getIsArrayEncodingT1())
    {
        if(!itsAfmEncoding.isEmpty() &&
#ifdef HAVE_FONT_ENC
           -1!=CGlobal::enc().getList().findIndex(itsAfmEncoding) &&
#else
           NULL!=CGlobal::enc().get8Bit(itsAfmEncoding) &&
#endif
           CEncodings::constT1Symbol!=itsAfmEncoding && 1==itsAfmEncoding.contains('-'))
            enc.append(itsAfmEncoding);

        enc.append(CEncodings::constT1Symbol);
    }
    else
        enc=getEncodingsFt();

    return enc;
}

bool CFontEngine::getIsArrayEncodingT1()
{
    return itsType==TYPE_1 && itsEncoding.find("array")!=-1 ? true : false;
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
          foundPs=false,
          familyIsFull=false;

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
            familyIsFull=true;
            foundFamily=true;
        }

        if(foundName)
            itsItalic=checkItalic(itsItalic, itsFullName);

        if(foundName && foundFamily)
            itsFamily=createNames(familyIsFull ? QString::null : itsFamily, itsFullName);
    }

    return foundPs && foundName && foundFamily;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TRUE TYPE
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontTT(const QString &file, unsigned short mask, int face)
{
    bool status=FT_New_Face(itsFt.library, QFile::encodeName(file), face, &itsFt.face) ? false : true;

    if(status)
    {
        itsFt.open=true;
        itsNumFaces=itsFt.face->num_faces;
    }

    if(TEST!=mask && status)
    {
        if(mask&NAME || mask&PROPERTIES)
        {
            bool famIsPs=false;

            itsFamily=getName(itsFt.face, TT_NAME_ID_FONT_FAMILY);
            itsFullName=getName(itsFt.face, TT_NAME_ID_FULL_NAME);

            if(itsFamily.isEmpty())
                if(itsFullName.isEmpty())
                {
                    famIsPs=true;
                    itsFamily=itsFullName=getName(itsFt.face, TT_NAME_ID_PS_NAME);
                }
                else
                    itsFamily=itsFullName;
            else
                if(itsFullName.isEmpty())
                    itsFullName=itsFamily;

            if(itsFullName.isEmpty())
                status=false;   // Hmm... couldn't find anby of the names!
            else
            {
                if(mask&PROPERTIES)
                {
                    void *table=NULL;
                    //
                    // Algorithm taken from ttf2pt1.c ...
                    //
                    QString psName=famIsPs ? itsFamily : getName(itsFt.face, TT_NAME_ID_PS_NAME);

                    if(psName.isEmpty())
                        psName=itsFullName;

                    itsPsName=psName;

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

                    bool gotItalic=false;

                    if(NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post)))
                    {
                        itsItalic=ITALIC_NONE;
                        itsItalicAngle=0;
                    }
                    else
                    {
                        struct TFixed
                        {
                             TFixed(unsigned long v) : upper(v>>16), lower(v&0xFFFF) {}

                             short upper,
                                   lower;

                             float value() { return upper+(lower/65536.0); }
                        };

                        gotItalic=true;
                        itsItalicAngle=((TFixed)((TT_Postscript*)table)->italicAngle).value();
                        itsItalic=itsItalicAngle== 0.0f ? ITALIC_NONE : ITALIC_ITALIC;
                    }

                    if((NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2))) || (0xFFFF==((TT_OS2*)table)->version) )
                    {
                        itsWidth=WIDTH_UNKNOWN;
                        itsWeight=WEIGHT_UNKNOWN;
                        if(!gotItalic)
                        {
                            itsItalicAngle=0;
                            itsItalic=ITALIC_NONE;
                        }
                    }
                    else
                    {
                        itsWeight=mapWeightTT(((TT_OS2*)table)->usWeightClass);
                        itsWidth=mapWidthTT(((TT_OS2*)table)->usWidthClass);
                        if(WEIGHT_UNKNOWN==itsWeight)
                            itsWeight=((TT_OS2*)table)->fsSelection&(1 << 5) ? WEIGHT_BOLD : WEIGHT_UNKNOWN;
                        if(!gotItalic)
                        {
                            itsItalic=((TT_OS2*)table)->fsSelection&(1 << 0) ? ITALIC_ITALIC : ITALIC_NONE;
                            itsItalicAngle=ITALIC_ITALIC==itsItalic ? -12 : 0 ; // Hmm...
                        }
                    }

                    if(WEIGHT_UNKNOWN==itsWeight && NULL!=(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head)))
                        itsWeight=((TT_Header *)table)->Mac_Style & 1 ? WEIGHT_BOLD : WEIGHT_UNKNOWN;

                    if(!gotItalic && NULL!=(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head)))
                    {
                        gotItalic=true;
                        itsItalic=((TT_Header *)table)->Mac_Style & 2 ? ITALIC_ITALIC: ITALIC_NONE; 
                    }

                    if(itsItalicAngle>45.0 || itsItalicAngle<-45.0)
                        itsItalicAngle=0.0;
                }

                if(mask&XLFD)
                {
                    void *table;

                    if(NULL!=(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post)) && ((TT_Postscript*)table)->isFixedPitch)
                        if(NULL!=(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_hhea)) &&
                           ((TT_HoriHeader *)table)->min_Left_Side_Bearing >= 0 && 
                           ((TT_HoriHeader *)table)->xMax_Extent <= ((TT_HoriHeader *)table)->advance_Width_Max)
                            itsSpacing=SPACING_CHARCELL;
                        else
                            itsSpacing=SPACING_MONOSPACED;
                    else
                        itsSpacing=SPACING_PROPORTIONAL;

                    itsFoundry=::getFoundry(itsFt.face);
                }
                if(mask&PROPERTIES || mask&XLFD || mask&NAME)
                    itsItalic=checkItalic(itsItalic, itsFullName);

                if(mask&NAME || mask&PROPERTIES)
                    itsFamily=createNames(itsFamily, itsFullName);
            }
        }
    }

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
    else if(weight<TTF_WEIGHT_NORMAL || weight<TTF_WEIGHT_MEDIUM)
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
        return FT_Select_Charmap(itsFt.face, ft_encoding_wansung) ?  FT_Select_Charmap(itsFt.face, ft_encoding_johab) ?  false : true : true;
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

#if ((FREETYPE_MAJOR > 2) || ((FREETYPE_MAJOR == 2) && (FREETYPE_MINOR >= 1)))
        if(FT_Has_PS_Glyph_Names(itsFt.face))
#else
        if(FT_HAS_GLYPH_NAMES(itsFt.face))
#endif
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
                                if((encoding->size <= 1 && failed > 0) || ((float)failed >= constBigEncodingFuzz * estimate))
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
#if ((FREETYPE_MAJOR > 2) || ((FREETYPE_MAJOR == 2) && (FREETYPE_MINOR >= 1)))
        return !found && FT_Has_PS_Glyph_Names(itsFt.face) ? true : false;
#else
        return !found && FT_HAS_GLYPH_NAMES(itsFt.face) ? true : false;
#endif

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

                itsFamily=createNames(itsFamily, itsFullName);

                itsPsName=constNoPsName;
                status=true;

                if(mask&NAME || mask&PROPERTIES)
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
                            itsWeight=WEIGHT_UNKNOWN;
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
                            itsWidth=WIDTH_UNKNOWN;
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

bool CFontEngine::openFontBmp(const QString &file, bool force)
{
    itsFoundry=constDefaultFoundry;

    QCString cFile(QFile::encodeName(file));

    if(isAPcf(cFile))
        return openFontPcf(file);
    if(isABdf(cFile))
        return openFontBdf(file);
    if(isASnf(cFile))
        return openFontSnf(file);

    return force 
               ? openFontPcf(file) || openFontBdf(file) || openFontSnf(file)
               : false;
}

void CFontEngine::createNameBmp(int pointSize, int res, const QString &enc)
{
    QString ptStr,
            resStr;

    ptStr.setNum(pointSize/10);
    resStr.setNum(res);
    itsFullName=itsFamily+" "+weightStr(itsWeight)+(ITALIC_ITALIC==itsItalic ? constBmpItalic : ITALIC_OBLIQUE==itsItalic ? constBmpOblique : constBmpRoman)+
                      " ("+ptStr+"pt, "+resStr+"dpi, "+enc+")";
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
        XLFD_UKNOWN,
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
            case XLFD_UKNOWN:
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

    if(keyLen+1<sLen && NULL!=(s=strstr(str, key)) && (s==str || (!isalnum(s[-1]) && '_'!=s[-1])) && (!noquotes || (noquotes && s[keyLen+1]=='-')))
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
            unsigned int numChars=((ntohl(genInfo.lastCol) - ntohl(genInfo.firstCol)) + 1) * ((ntohl(genInfo.lastRow) - ntohl(genInfo.firstRow)) + 1),
                         glyphInfoSize=((genInfo.maxBounds.byteOffset()+3) & ~0x3);

            if(props)
            {
                if(-1!=snf.seek(numChars*sizeof(TCharInfo)+glyphInfoSize, SEEK_CUR))  // Skip past character info & glyphs...
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

bool CFontEngine::getGlyphBitmap(FTC_Image_Desc &font, FT_ULong index, Bitmap &target, int &left, int &top,
                                 int &xAdvance, FT_Pointer *ptr)
{
    bool ok=false;

    *ptr=NULL;

    //
    // Cache small glyphs, else render on demand...
    if(font.font.pix_width<48 && font.font.pix_height<48)
    {
        FTC_SBit sbit;

        if(!FTC_SBit_Cache_Lookup(itsFt.sBitCache, &font, index, &sbit))
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
    else
    {
        FT_Glyph glyph;

        if(!FTC_Image_Cache_Lookup(itsFt.imageCache, &font, index, &glyph))
        {
            ok=true;

            if(ft_glyph_format_outline==glyph->format)
                if(ok=!FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, NULL, 0))
                    *ptr=glyph;

            if(ok)
                if(ft_glyph_format_bitmap==glyph->format)
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

bool CFontEngine::drawGlyph(QPixmap &pix, FTC_Image_Desc &font, int glyphNum,
                            FT_F26Dot6 &x, FT_F26Dot6 &y, FT_F26Dot6 width, FT_F26Dot6 height,
                            FT_F26Dot6 startX, FT_F26Dot6 stepY, int space)
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

        QPixmap glyphPix(QImage(bmp.buffer, bmp.width, bmp.height, bmp.mono ? 1 : 8, bmp.mono ? clut1 : clut8, bmp.mono ? 2 : bmp.greys,
                                QImage::BigEndian));

        bitBlt(&pix, x+left, y-top, &glyphPix, 0, 0, glyphPix.width(), glyphPix.height(), Qt::AndROP);

        if(glyph)
            FT_Done_Glyph((FT_Glyph)glyph);

        x+=xAdvance+1;
    }
    else if(x!=startX)
        x+=space;

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
    if(FTC_SBit_Cache_New(cacheManager, &sBitCache))
    {
        std::cerr << "ERROR: Could not initliaze FreeType2 small bitmaps cache\n";
        exit(0);
    }
    if(FTC_Image_Cache_New(cacheManager, &imageCache))
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
