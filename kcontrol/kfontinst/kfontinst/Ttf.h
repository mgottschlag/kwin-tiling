#ifndef __TTF_H__
#define __TTF_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CTtf
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst)
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

#include <stdio.h>
#include <qglobal.h>
#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#endif
#include <qstring.h>

#include <fstream>

class CTtf
{
    private:

    struct TPsNameMap
    {
        unsigned long unicode;
        QString       psName;
    };

    struct TFixed
    {
        short          upper;
        unsigned short lower;
    };
 
    typedef struct THead
    {
        unsigned long  version,
                       fontRevision,
                       checksumAdjust,
                       magicNo;
        unsigned short flags,
                       unitsPerEm;
        unsigned char  created[8],
                       modified[8];
        short          xMin,
                       yMin,
                       xMax,
                       yMax;
        unsigned short macStyle,
                       lowestRecPPEM;
        short          fontDirection,
                       indexToLocFormat,
                       glyphDataFormat;
    };
 
    struct TDirEntry
    {
        char          tag[4];
        unsigned long checksum,
                      offset,
                      length;
    };
 
    struct TDirectory
    {
        unsigned long  sfntVersion;
        unsigned short numTables,
                       searchRange,
                       entrySelector,
                       rangeShift;
        TDirEntry      list;
    };
 
    struct TPostHead
    {
        unsigned long  formatType;
        TFixed         italicAngle;
        short          underlinePosition,
                       underlineThickness;
        unsigned long  isFixedPitch,
                       minMemType42,
                       maxMemType42,
                       minMemType1,
                       maxMemType1;
        unsigned short numGlyphs,
                       glyphNameIndex;
    };

    struct TKern
    {
        unsigned short version,
                       numSubTables;
    };

    struct TKernSubTable
    {
        unsigned short version,
                       length,
                       coverage;
    };

    struct TKernFmt0Header
    {
        unsigned short numPairs,
                       searchRange,
                       entrySelector,
                       rangeShift;
    };

    struct TKernFmt0
    {
        unsigned short left,
                       right;
        short          value;
    };

/*
    struct TKernFmt2Header
    {
        unsigned short rowWidth,
                       leftClassTable,
                       rightClassTable,
                       arrayOffset;
    };

    struct TKernFmt2ArrayHeader
    {
        unsigned short firstGlyph,
                       numGlyphs;
    };

    struct TKernFmt2
    {
        unsigned short left,
                       right;
    };
*/

    public:

    struct TKerning
    {
        TKerning(unsigned long l, unsigned long r, short v) : left(l), right(r), value(v) {}

        unsigned long left,
                      right;
        short         value;
    };

    enum EStatus
    {
        SUCCESS =0,
        FILE_OPEN_ERROR,
        FILE_WRITE_ERROR,
        NO_POST,
        NO_HEAD,
        NO_CMAP,
        NO_SUITABLE_TABLE,
        USES_MAC_STANDARD,
        CHANGE_MADE,
        CONFIG_FILE_NOT_OPENED,
        NO_REMAP_GLYPHS,
        FILE_FORMAT_ERROR
    }; 

    public:

    CTtf();
    ~CTtf();

    EStatus fixPsNames(const QString &nameAndPath);

#if QT_VERSION >= 300
    static QPtrList<TKerning> * getKerningData(const QString &nameAndPath);
#else
    static QList<TKerning> * getKerningData(const QString &nameAndPath);
#endif
    static QString           toString(EStatus status);

    private:

    static bool   locateTable(std::ifstream &ttf, const char *table);
    EStatus       readFile(const QString &nameAndPath);
    EStatus       writeFile(const QString &nameAndPath);
    EStatus       fixGlyphName(int index, const char *newName);
    unsigned long checksum(unsigned long *tbl, unsigned long numb);
    EStatus       checksum();

    private:

    char              *itsBuffer;
    unsigned int      itsBufferSize;
    QList<TPsNameMap> itsPsNameList;
};

#endif
