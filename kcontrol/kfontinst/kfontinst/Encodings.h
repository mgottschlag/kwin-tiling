#ifndef __ENCODINGS_H__
#define __ENCODINGS_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CEncodings
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include <qstring.h>
#include <qglobal.h>
#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#endif

class CEncodings
{
    public:

    struct T8Bit
    {
        enum EConstants
        {
            INDEX_OFFSET=32,
            NUM_MAP_ENTRIES = 256-INDEX_OFFSET
        };
 
        T8Bit(const QString &f, const QString &n, int *m=NULL) : file(f), name(n), map(m) {}
        virtual ~T8Bit();
 
        bool load();  // Load data in from file (if applicable...)
        QString file,
                name;
        int     *map;
    };

    struct T16Bit
    {
        T16Bit(const QString &f, const QString &n) : file(f), name(n) {}

        QString file,
                name;
    };

    enum EConstants
    {
        MS_SYMBOL_MODIFIER = 0xf020   // Charmap for micorosft-symbol (i.e. add this value...)
    };

    CEncodings();
    virtual ~CEncodings()                               { }

    void                  reset();
    bool                  createEncodingsDotDir(const QString &dir);
    void                  addDir(const QString &path)   { addDir(path, 0); }
    void                  clear()                       { its8BitList.clear(); its16BitList.clear(); }
#if QT_VERSION >= 300
    const QPtrList<T8Bit> & list8Bit()                  { return its8BitList; }
#else
    const QList<T8Bit> &  list8Bit()                    { return its8BitList; }
#endif
    T8Bit *               first8Bit()                   { return its8BitList.first(); }
    T8Bit *               next8Bit()                    { return its8BitList.next(); }
#if QT_VERSION >= 300
    const QPtrList<T16Bit> & list16Bit()                { return its16BitList; }
#else
    const QList<T16Bit> & list16Bit()                   { return its16BitList; }
#endif
    T16Bit *              first16Bit()                  { return its16BitList.first(); }
    T16Bit *              next16Bit()                   { return its16BitList.next(); }
    T8Bit *               get8Bit(const QString &enc);
    QString               getFile8Bit(const QString &enc);
    static bool           isBuiltin(const T8Bit &enc);
    static bool           isAEncFile(const char *file);
    static bool           isUnicode(const QString &enc) { return strcmp(constUnicodeStr.latin1(), enc.latin1())==0 ? true : false; }

    static const QString constUnicodeStr;
    static const QString constT1Symbol;
    static const QString constTTSymbol;

    private:

    void                 addDir(const QString &path, int sub);

#if QT_VERSION >= 300
    QPtrList<T8Bit>  its8BitList;
    QPtrList<T16Bit> its16BitList;
#else
    QList<T8Bit>  its8BitList;
    QList<T16Bit> its16BitList;
#endif
    unsigned int  itsNumBuiltin;
};

#endif
