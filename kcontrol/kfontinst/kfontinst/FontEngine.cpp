///////////////////////////////////////////////////////////////////////////////
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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
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
#include "CompressedFile.h"
#include "Misc.h"
#include <kglobal.h>
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

static const char *constNoPsName             = "NO_PS_NAME";
static const char *constBmpRoman             = "";
static const char *constBmpItalic            = " Italic";
static const char *constBmpOblique           = " Oblique";
static const char *constOblique              = "Oblique";
static const char *constSlanted              = "Slanted";
static const char *const constDefaultFoundry = "Misc";

namespace KFI
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    GENERIC
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_FONT_ENC
CFontEngine::~CFontEngine()
{
    closeFont();
    delete itsEncodings;
}

CEncodings * CFontEngine::encodings()
{
    if(!itsEncodings)
        itsEncodings=new CEncodings();
    return itsEncodings;
}
#endif

bool CFontEngine::openFont(const QString &file, int face)
{
    bool ok=false;

    if(file==itsPath && face==itsFaceIndex)
    {
        ok=NONE!=itsType;
    }
    else
    {
        closeFont();

        itsWeight=WEIGHT_MEDIUM;
        itsWidth=WIDTH_NORMAL;
        itsSpacing=SPACING_PROPORTIONAL;
        itsItalic=ITALIC_NONE;
        itsAddStyle=QString::null;
        itsPath=file;
        itsFaceIndex=face;

        if(!openFontFt(file) && !openFontPcf(file) && !openFontSpd(file) && !openFontBdf(file) && !openFontSnf(file))
            itsType=NONE;
    }

    return NONE!=itsType;
}

void CFontEngine::closeFont()
{
    closeFaceFt();
    itsPath=QString::null;
    itsFaceIndex=-1;
    itsType=NONE;
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
        case TYPE_1:
            return itsFt.open && FT_IS_SCALABLE(itsFt.face);
        case SPEEDO:
            return true;
        case BITMAP:
        default:
            return false;
    }
}

QStringList CFontEngine::getEncodings()
{
    switch(itsType)
    {
        case TRUE_TYPE:
        case TYPE_1:
            return getEncodingsFt();
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
        return WEIGHT_MEDIUM; // WEIGHT_UNKNOWN;
    else if(kasciistricmp(str, "Bold")==0)
        return WEIGHT_BOLD;
    else if(kasciistricmp(str, "Black")==0)
        return WEIGHT_BLACK;
    else if(kasciistricmp(str, "ExtraBold")==0)
        return WEIGHT_EXTRA_BOLD;
    else if(kasciistricmp(str, "UltraBold")==0)
        return WEIGHT_ULTRA_BOLD;
    else if(kasciistricmp(str, "ExtraLight")==0)
        return WEIGHT_EXTRA_LIGHT;
    else if(kasciistricmp(str, "UltraLight")==0)
        return WEIGHT_ULTRA_LIGHT;
    else if(kasciistricmp(str, "Light")==0)
        return WEIGHT_LIGHT;
    else if(kasciistricmp(str, "Medium")==0 || kasciistricmp(str, "Normal")==0 || kasciistricmp(str, "Roman")==0)
        return WEIGHT_MEDIUM;
    else if(kasciistricmp(str, "Regular")==0)
        return WEIGHT_MEDIUM; // WEIGHT_REGULAR;
    else if(kasciistricmp(str, "Demi")==0)
        return WEIGHT_DEMI;
    else if(kasciistricmp(str, "SemiBold")==0)
        return WEIGHT_SEMI_BOLD;
    else if(kasciistricmp(str, "DemiBold")==0)
        return WEIGHT_DEMI_BOLD;
    else if(kasciistricmp(str, "Thin")==0)
        return WEIGHT_THIN;
    else if(kasciistricmp(str, "Book")==0)
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
//    Type1 & TrueType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontFt(const QString &file)
{
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

    bool status=FT_New_Face(itsFt.library, QFile::encodeName(file), 0, &itsFt.face) ? false : true;

    if(status)
        itsFt.open=true;

    PS_FontInfoRec t1info;

    if(0==FT_Get_PS_Font_Info(itsFt.face, &t1info))
    {
        itsFullName=t1info.full_name;
        itsFamily=t1info.family_name;
        itsType=TYPE_1;
    }
    else
    {
        itsFullName=getName(itsFt.face, TT_NAME_ID_FULL_NAME);
        itsFamily=getName(itsFt.face, TT_NAME_ID_FONT_FAMILY);
        itsType=TRUE_TYPE;
    }

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

    if(status)
    {
        removeSymbols(itsFullName);
        removeSymbols(itsFamily);
        setPsNameFt();

        if(TYPE_1==itsType)
        {
            itsWeight=strToWeight(t1info.weight);
            itsItalic=t1info.italic_angle <= -4 || t1info.italic_angle >= 4 ? ITALIC_ITALIC : ITALIC_NONE;
            itsWidth=strToWidth(itsFullName);
            itsItalic=checkItalic(itsItalic, itsFullName);
            itsSpacing=t1info.is_fixed_pitch ? SPACING_MONOSPACED : SPACING_PROPORTIONAL;
            itsFoundry=KFI::getFoundry(t1info.notice);
        }
        else // TrueType...
        {
            TT_Postscript *post=NULL;
            TT_OS2        *os2=NULL;
            TT_Header     *head=NULL;
            bool          gotItalic=false;

            if(NULL==(os2=(TT_OS2 *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2)) || 0xFFFF==os2->version)
            {
                itsWidth=WIDTH_UNKNOWN;
                itsWeight=WEIGHT_UNKNOWN;
            }
            else
            {
                FT_UShort weight=(os2->usWeightClass>0 && os2->usWeightClass<100) ? os2->usWeightClass*100 : os2->usWeightClass;

                if(weight<TTF_WEIGHT_THIN)
                    itsWeight=WEIGHT_THIN;
                else if(weight<TTF_WEIGHT_EXTRALIGHT)
                    itsWeight=WEIGHT_EXTRA_LIGHT;
                else if(weight<TTF_WEIGHT_LIGHT)
                    itsWeight=WEIGHT_LIGHT;
                else if(/*weight<TTF_WEIGHT_NORMAL || */ weight<TTF_WEIGHT_MEDIUM)
                    itsWeight=WEIGHT_MEDIUM;
                else if(weight<TTF_WEIGHT_SEMIBOLD)
                    itsWeight=WEIGHT_SEMI_BOLD;
                else if(weight<TTF_WEIGHT_BOLD)
                    itsWeight=WEIGHT_BOLD;
                else if(weight<TTF_WEIGHT_EXTRABOLD)
                    itsWeight=WEIGHT_EXTRA_BOLD;
                else if(weight<TTF_WEIGHT_BLACK)
                    itsWeight=WEIGHT_BLACK;
                else if(os2->fsSelection&(1 << 5))
                    itsWeight=WEIGHT_BOLD;
                else
                    itsWeight=WEIGHT_UNKNOWN;

                switch(os2->usWidthClass)
                {
                    case TTF_WIDTH_ULTRA_CONDENSED:
                        itsWidth=WIDTH_ULTRA_CONDENSED;
                    case TTF_WIDTH_EXTRA_CONDENSED:
                        itsWidth=WIDTH_EXTRA_CONDENSED;
                    case TTF_WIDTH_CONDENSED:
                        itsWidth=WIDTH_CONDENSED;
                    case TTF_WIDTH_SEMI_CONDENSED:
                        itsWidth=WIDTH_SEMI_CONDENSED;
                    case TTF_WIDTH_NORMAL:
                        itsWidth=WIDTH_NORMAL;
                    case TTF_WIDTH_SEMI_EXPANDED:
                        itsWidth=WIDTH_SEMI_EXPANDED;
                    case TTF_WIDTH_EXPANDED:
                        itsWidth=WIDTH_EXPANDED;
                    case TTF_WIDTH_EXTRA_EXPANDED:
                        itsWidth=WIDTH_EXTRA_EXPANDED;
                    case TTF_WIDTH_ULTRA_EXPANDED:
                        itsWidth=WIDTH_ULTRA_EXPANDED;
                    default:
                        itsWidth=WIDTH_UNKNOWN;
                }

                itsItalic=os2->fsSelection&(1 << 0) ? ITALIC_ITALIC : ITALIC_NONE;
                gotItalic=true;
            }

            if(WEIGHT_UNKNOWN==itsWeight)
                itsWeight=NULL!=(head=(TT_Header *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head)) && head->Mac_Style & 1
                             ? WEIGHT_BOLD
                             : WEIGHT_MEDIUM;

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

            itsItalic=checkItalic(itsItalic, itsFullName);

            if((NULL!=post || NULL!=(post=(TT_Postscript *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post))) &&  post->isFixedPitch)
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

            itsFoundry=KFI::getFoundry(itsFt.face, os2);
        }

        createAddStyle();
    }

    if(!status)
        closeFaceFt();

    return status;
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
bool CFontEngine::has8BitEncodingFt(const CEncodings::T8Bit &data)
{
    int cm;

    for(cm=0; cm<itsFt.face->num_charmaps; cm++)
    {
        static const int constMaxMissing=5;

        int ch,
            missing=0;

        setCharmapFt(itsFt.face->charmaps[cm]);

        for(ch=0; ch<CEncodings::T8Bit::NUM_MAP_ENTRIES && missing<=constMaxMissing; ch++)
            if(data.map[ch]>-1 && FT_Get_Char_Index(itsFt.face, data.map[ch])==0)
                missing++;

        if(missing<=constMaxMissing)
            return true;
    }

    return false;
}

QStringList CFontEngine::get8BitEncodingsFt()
{
    QStringList enc;

    for(int i=0; CEncodings::eightBit()[i].map; i++)
        if(has8BitEncodingFt(CEncodings::eightBit()[i]))
            enc.append(CEncodings::eightBit()[i].name);

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
            break;   // CPD TODO??? This means the following is NOT used???
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

    FontEncPtr encoding=FontEncFind(enc.latin1(), NULL);   // CPD TODO latin1 ???

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
        QStringList::ConstIterator it,
                                   end=encodings()->getList().end();
        bool                       found=false;

        for(it=encodings()->getList().begin(); it!=end; ++it)
            if(checkEncodingFt(*it))
            {
                enc.append(*it);
                found=true;
            }

        end=encodings()->getExtraList().end();

        for(it=encodings()->getExtraList().begin(); it!=end; ++it)
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

        if(TRUE_TYPE==itsType)
        {
            if(FT_Err_Ok==FT_Select_Charmap(itsFt.face, ft_encoding_sjis))
            {
                enc.append("jisx0208.1983-0");
                enc.append("jisx0201.1976-0");
            }
            if(FT_Err_Ok==FT_Select_Charmap(itsFt.face, ft_encoding_gb2312))
                enc.append("gb2312.1980-0");
            if(FT_Err_Ok==FT_Select_Charmap(itsFt.face, ft_encoding_big5))
                enc.append("big5.et-0");
            if(FT_Err_Ok==FT_Select_Charmap(itsFt.face, ft_encoding_wansung) ||
               FT_Err_Ok==FT_Select_Charmap(itsFt.face, ft_encoding_johab) )
                enc.append("ksc5601.1987-0");
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
bool CFontEngine::openFontSpd(const QString &file)
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
                itsSpacing=hdr[constSpacingOffset]==constMonospaced ? SPACING_MONOSPACED : SPACING_PROPORTIONAL;
                hdr[constNoticeOffset+constNoticeNumBytes]='\0';
                itsFoundry=KFI::getFoundry((const char *)&hdr[constNoticeOffset]);
            }
        }
        spd.close();
    }

    if(status)
        itsType=SPEEDO;

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
                    itsFullName=str;
                    foundXlfd=true;
                }
                break;
            }
        }
    }

    if(foundXlfd)
        itsType=BITMAP;

    return foundXlfd;
}

static const char * readStrSnf(CCompressedFile &f)
{
    static const int constMaxChars=512;

    static char buffer[constMaxChars];
    int         pos=0;
    signed char        ch;

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

                                    if(!foundXlfd && kasciistricmp(name, "FONT")==0 && strlen(value))
                                    {
                                        foundXlfd=true;
                                        itsFullName=value;
                                    }
                                }
                                else
                                    break;
                    }
                }
                delete [] props;
            }
        }
    }

    if(foundXlfd)
        itsType=BITMAP;

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
                                                if(kasciistricmp(&str[props[prop].name], "FONT")==0)
                                                {
                                                    if(props[prop].isString && strlen(&str[props[prop].value]))
                                                    {
                                                        foundXlfd=true;
                                                        strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                        tmp[constMaxStrLen-1]='\0';
                                                        itsFullName=tmp;
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
    }

    if(foundXlfd)
        itsType=BITMAP;

    return foundXlfd;
}

CFontEngine::TFtData::TFtData()
                    : open(false)
{
    if(FT_Init_FreeType(&library))
    {
        std::cerr << "ERROR: FreeType2 failed to initialise\n";
        exit(0);
    }
}

CFontEngine::TFtData::~TFtData()
{
    FT_Done_FreeType(library);
}

}
