////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CAfmCreator
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 02/05/2001
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "AfmCreator.h"
#include "Misc.h"
#include "FontEngine.h"
#include "Config.h"
#include "Kfi.h"
#include "KfiGlobal.h"
#include "Ttf.h"
#include <stdlib.h>
#include <qstring.h>
#include <qdir.h>
#include <qvaluelist.h>
#include <qglobal.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <stdio.h>
#include <string.h>
#include <fstream.h>

static const char * constNotDef     = ".notdef";
static const char * constKfiComment = "Comment kfontinst ";

using namespace std;

CAfmCreator::EStatus CAfmCreator::go(const QString &dir)
{
    EStatus status=SUCCESS;
    QDir    d(dir);

    if(d.isReadable())
    {
        const QFileInfoList *files=d.entryInfoList();
        EStatus             st=SUCCESS;
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()) && SUCCESS==status; ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName() &&
                   ((CFontEngine::isAType1(fInfo->fileName().local8Bit()) && CKfiGlobal::cfg().getDoT1Afms()) ||
                    (CFontEngine::isATtf(fInfo->fileName().local8Bit()) && CKfiGlobal::cfg().getDoTtAfms()) ))
                {
                    bool createAfm = CMisc::fExists(CMisc::afmName(fInfo->filePath())) 
                                         ? false 
                                         : true;

                    emit step(i18n("Creating AFM: ")+CMisc::afmName(fInfo->filePath())); 

                    if(createAfm && (st=create(fInfo->filePath()))!=SUCCESS && KMessageBox::questionYesNo(NULL,
                                                                       i18n("There was an error creating:\n%1\nDo you wish to continue?").
                                                                       arg(CMisc::afmName(fInfo->fileName())), i18n("AFM Error"))==KMessageBox::No)
                            status=st;
                }
        }
    }

    return status;
}

CAfmCreator::EStatus CAfmCreator::create(const QString &fName)
{
    EStatus status=SUCCESS;

    if(CKfiGlobal::fe().openFont(fName, CFontEngine::AFM))
    {
        if(CKfiGlobal::fe().setCharmapSymbolFt() || CKfiGlobal::fe().getIsArrayEncodingT1() || !CKfiGlobal::fe().setCharmapUnicodeFt()) // Then its a (/use) symbol encoding...
            status=create(fName, CFontEngine::isAType1(fName.local8Bit()) ? CEncodings::constT1Symbol : CEncodings::constTTSymbol, true);
        else
        {
#if QT_VERSION >= 300
            const char * constDefaultCharSet = "iso8859-1";
#endif
            QStringList encs=CKfiGlobal::fe().get8BitEncodings();
            QString     enc;

            if(encs.count())
                if(encs.findIndex(CKfiGlobal::cfg().getAfmEncoding())!=-1)   // Check for requested encoding...
                    enc=CKfiGlobal::cfg().getAfmEncoding();
                else
#if QT_VERSION < 300
                    if(encs.findIndex(QFont::encodingName(QFont::charSetForLocale()))!=-1)   // Try locale...
                        enc=QFont::encodingName(QFont::charSetForLocale());
#else
                    if(encs.findIndex(constDefaultCharSet))
                        enc=constDefaultCharSet;
#endif
                    else
                        enc=encs.first();  // Hmmm... just use the first available...

            if(QString::null!=enc)
                status=create(fName, enc, CEncodings::constT1Symbol==enc || CEncodings::constTTSymbol==enc ? true : false);
            else
                status=COULD_NOT_FIND_ENCODING;
        }
        CKfiGlobal::fe().closeFont();
    }
    else
        status=COULD_NOT_OPEN_FONT;

    return status;
}

inline void getGlyphMetrics(QStringList &list, int charCode, int glyph, int xMin, int xMax, int yMin, int yMax, bool useNotDefs=false)
{
    const CFontEngine::TGlyphInfo *inf=CKfiGlobal::fe().getGlyphInfo(glyph);
 
    if(NULL!=inf || (NULL!=(inf=CKfiGlobal::fe().getGlyphInfo(0))))
        if('\0'!=inf->name[0] && (useNotDefs || strcmp(inf->name, constNotDef)!=0))
        {
            QCString entry,
                     num;
            // Quick hack - make sure character's bbox is within main bbox
            int cxMin=inf->xMin < xMin ? xMin : inf->xMin,
                cxMax=inf->xMax > xMax ? xMax : inf->xMax,
                cyMin=inf->yMin < yMin ? yMin : inf->yMin,
                cyMax=inf->yMax > yMax ? yMax : inf->yMax;

            entry+="C ";
            entry+=num.setNum(charCode);
            entry+=" ; WX ";
            entry+=num.setNum(inf->scaledWidth);
            entry+=" ; N ";
            if('\0'==inf->name[0] || strcmp(inf->name, constNotDef)==0)
                entry+=constNotDef;
            else
            {
                entry+=inf->name;
                entry+=" ; B ";
                entry+=num.setNum(cxMin);
                entry+=' ';
                entry+=num.setNum(cyMin);
                entry+=' ';
                entry+=num.setNum(cxMax);
                entry+=' ';
                entry+=num.setNum(cyMax);
            }
            entry+=" ;";
            list.append(entry);
        }
}

CAfmCreator::EStatus CAfmCreator::create(const QString &fName, const QString &encoding, bool symEnc)
{
    QString afmName=CMisc::afmName(fName);
    EStatus status=SUCCESS;
    
    //
    // Only create an AFM file, if it doesnt already exist - or it has a different encoding...
    //
    if(!CMisc::fExists(afmName) || encoding!=getEncoding(afmName))
    {
        CEncodings::T8Bit *enc=NULL; // CKfiGlobal::enc().get8Bit(CKfiGlobal::cfg().getAfmEncoding());
#if QT_VERSION >= 300
        QPtrList<TKerning> kerning;
#else
        QList<TKerning>   kerning;
#endif
        QStringList       composite,
                          chars;
        QValueList<int>   usedGlyphs;

        // Set charmap...
        if(!symEnc)
        {
            if(NULL!=(enc=CKfiGlobal::enc().get8Bit(encoding)) && CKfiGlobal::fe().setCharmapUnicodeFt())
                enc->load();
            else
            {
                status=COULD_NOT_FIND_ENCODING;
                return status;
            }
        }
        else
            if(!CKfiGlobal::fe().setCharmapSymbolFt() && !CFontEngine::isAType1(fName.local8Bit()))
            {
                status=COULD_NOT_FIND_ENCODING;
                return status;
            }

        //
        // Get the Kerning and Composite data for the font...
        //     For Type1 this is read in from any existing AFM file
        //     For TrueType only the Kerning data is obtained - and this is read from the .ttf file
        //
        readKerningAndComposite(fName, kerning, composite, enc);  // Must do this before we overwrite the file below...

        ofstream afm(afmName.local8Bit());

        if(afm)
        {
            unsigned int ch;
            int          xMin=CKfiGlobal::fe().getBBoxXMin(),
                         xMax=CKfiGlobal::fe().getBBoxXMax(),
                         yMin=CKfiGlobal::fe().getBBoxYMin(),
                         yMax=CKfiGlobal::fe().getBBoxYMax(),
                         glyph;

            afm << "StartFontMetrics 2.0" << endl
                << constKfiComment << encoding.latin1() << endl
                << "FontName " << CKfiGlobal::fe().getPsName().latin1() << endl
                << "FullName " << CKfiGlobal::fe().getFullName().latin1() << endl
                << "FamilyName " << CKfiGlobal::fe().getFamilyName().latin1() << endl
                << "Weight " << CFontEngine::weightStr(CKfiGlobal::fe().getWeight()).latin1() << endl
                << "Notice Created with kfontinst v" << CKfi::constVersion << endl
                << "ItalicAngle " << CKfiGlobal::fe().getItalicAngle() << endl
                << "IsFixedPitch " << (CFontEngine::SPACING_MONOSPACED==CKfiGlobal::fe().getSpacing() ? "true" : "false") << endl
                << "UnderlinePosition " << CKfiGlobal::fe().getUnderlinePosition() << endl
                << "UnderlineThickness " << CKfiGlobal::fe().getUnderlineThickness() << endl 
                << "Version 001.00" << endl
                << "EncodingScheme ";

            if(CFontEngine::isAType1(fName.local8Bit()) && QString::null!=CKfiGlobal::fe().getAfmEncodingT1())
                afm << CKfiGlobal::fe().getAfmEncodingT1().latin1();
            else
                afm << "FontSpecific";

            afm << endl
                << "FontBBox " << xMin << ' ' << yMin << ' ' << xMax << ' ' << yMax << endl
                << "Descender " << CKfiGlobal::fe().getDescender() << endl
                << "Ascender " << CKfiGlobal::fe().getAscender() << endl;

                for(ch=0; ch<CEncodings::T8Bit::NUM_MAP_ENTRIES; ++ch)
                {
                    unsigned short code=symEnc ?
                                            ( CKfiGlobal::fe().getType()==CFontEngine::TRUE_TYPE ?
                                                  CKfiGlobal::fe().getGlyphIndexFt(ch+CEncodings::MS_SYMBOL_MODIFIER) :
                                                  ch
                                            ) :
                                            CKfiGlobal::fe().getGlyphIndexFt(enc->map[ch]);

                    if(!usedGlyphs.contains((int)code))
                        usedGlyphs.append(code);

                    getGlyphMetrics(chars, ch+CEncodings::T8Bit::INDEX_OFFSET, code, xMin, xMax, yMin, yMax);
                }

                for(glyph=1; glyph<CKfiGlobal::fe().getNumGlyphsFt(); ++glyph)  // Glyph 0 == .null
                    if(!usedGlyphs.contains(glyph))
                        getGlyphMetrics(chars, -1, glyph, xMin, xMax, yMin, yMax);

                afm << "StartCharMetrics " << chars.count() << endl;

                if(chars.count())
                {
                    QStringList::Iterator it;
 
                    for(it=chars.begin(); it!=chars.end(); ++it)
                        afm << (*it).latin1() << endl;
                }

                afm << "EndCharMetrics" << endl;

                if(kerning.count())
                { 
                    afm << "StartKernData" << endl
                        << "StartKernPairs " << kerning.count() << endl;

                    TKerning *kern;
 
                    for(kern=kerning.first(); NULL!=kern; kern=kerning.next())
                        afm << "KPX " << kern->left.latin1() << ' ' << kern->right.latin1() << ' ' << kern->value << endl;

                    afm << "EndKernPairs" << endl
                        << "EndKernData" << endl;
                }

                if(composite.count())
                {
                    afm << "StartComposites" << composite.count() << endl;
 
                    QStringList::Iterator it;
 
                    for(it=composite.begin(); it!=composite.end(); ++it)
                        afm << (*it).latin1() << endl;
 
                    afm << "EndComposites" << endl;
                }

                afm << "EndFontMetrics" << endl;

                afm.close();
        }
        else
            status=COLD_NOT_CREATE_FILE;
    }

    return status;
} 

QString CAfmCreator::getEncoding(const QString &afm)
{
    //
    // Return the 'kfontinst' encoding of the AFM file...
    //
    QString  enc;
    ifstream f(afm.local8Bit());
 
    if(f)
    {
        const int constMaxLen=512;
 
        char  line[constMaxLen];
 
        do
        {
            f.getline(line, constMaxLen);
 
            if(f.good())
            {
                line[constMaxLen-1]='\0';
                if(strstr(line, constKfiComment)==line)
                {
                    enc=&line[strlen(constKfiComment)];
                    break;
                }
            }
        }
        while(!f.eof());
        f.close();
    }

    return enc;
}

static bool encContainsGlyph(CEncodings::T8Bit &enc, unsigned int glyph)
{
    int ch;

    for(ch=0; ch<CEncodings::T8Bit::NUM_MAP_ENTRIES; ++ch)
        if(CKfiGlobal::fe().getGlyphIndexFt(enc.map[ch])==glyph)
            return true;
    return false;
}

#if QT_VERSION >= 300
void CAfmCreator::readKerningAndComposite(const QString &font, QPtrList<TKerning> &kern, QStringList &comp, CEncodings::T8Bit *enc)
#else
void CAfmCreator::readKerningAndComposite(const QString &font, QList<TKerning> &kern, QStringList &comp, CEncodings::T8Bit *enc)
#endif
{
    if(CFontEngine::isAType1(font.local8Bit()))
    {
        ifstream f(CMisc::afmName(font).local8Bit());
 
        if(f)
        {
            const int constMaxLen=512;

            char  line[constMaxLen],
                  left[constMaxLen],
                  right[constMaxLen];
            short value;

            do
            {
                f.getline(line, constMaxLen);

                if(f.good())
                {
                    line[constMaxLen-1]='\0';
                    if(strstr(line, "KPX")==line)
                    {
                        if(sscanf(line, "KPX %s %s %hi", left, right, &value)==3)
                            kern.append(new TKerning(left, right, value));
                    }
                    else if(strstr(line, "CC ")==line)
                        comp.append(line);
                }
            }
            while(!f.eof());
            f.close();
        }
    }
    else
        if(NULL!=enc && CFontEngine::isATtf(font.local8Bit()))
        {
#if QT_VERSION >= 300
            QPtrList<CTtf::TKerning> *ttfList=CTtf::getKerningData(font);
#else
            QList<CTtf::TKerning> *ttfList=CTtf::getKerningData(font);
#endif
            CTtf::TKerning        *kd;

            if(ttfList)
            {
                for(kd=ttfList->first(); NULL!=kd; kd=ttfList->next())
                    if(encContainsGlyph(*enc, kd->left) && encContainsGlyph(*enc, kd->right))
                    {
                        const CFontEngine::TGlyphInfo *inf;

                        inf=CKfiGlobal::fe().getGlyphInfo(kd->left);
                        if('\0'!=inf->name[0] && strcmp(inf->name, constNotDef)!=0)
                        {
                            QString ln=inf->name;

                            inf=CKfiGlobal::fe().getGlyphInfo(kd->right);
                            if('\0'!=inf->name[0] && strcmp(inf->name, constNotDef)!=0)
                                kern.append(new TKerning(ln, inf->name, CKfiGlobal::fe().scaleMetric(kd->value)));
                        }
                    }
                delete ttfList;
            }
        }
}

QString CAfmCreator::statusToStr(EStatus val)
{
    switch(val)
    {
        case SUCCESS:
            return i18n("Success");
        case COULD_NOT_FIND_ENCODING:
            return i18n("Could not find encoding");
        case COULD_NOT_OPEN_FONT:
            return i18n("Could not open font file");
        case COLD_NOT_CREATE_FILE:
            return i18n("Could not create file");
        default:
            return i18n("Unknown");
    }
}
#include "AfmCreator.moc"
