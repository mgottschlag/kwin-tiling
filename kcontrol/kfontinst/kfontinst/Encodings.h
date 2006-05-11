#ifndef __ENCODINGS_H__
#define __ENCODINGS_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CEncodings
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QString>

#ifdef HAVE_FONT_ENC
#include <qstringlist.h>
#endif

namespace KFI
{

class CEncodings
{
    public:

#ifndef HAVE_FONT_ENC
    struct T8Bit
    {
        enum EConstants
        {
            INDEX_OFFSET=32,
            NUM_MAP_ENTRIES = 256-INDEX_OFFSET
        };
 
        const char *name;
        int        *map;
    };
#endif

#ifdef HAVE_FONT_ENC
    CEncodings();
    virtual ~CEncodings()                                  { }
#endif

#ifdef HAVE_FONT_ENC
    static bool              createEncodingsDotDir(const QString &dir);

    const QStringList &      getList()                     { return itsList; }
    const QStringList &      getExtraList()                { return itsExtraList; }
#else
    static const T8Bit *     eightBit()                    { return their8Bit; }
#endif

    static const QString constUnicode;
    static const QString constT1Symbol;
    static const QString constTTSymbol;

#ifndef HAVE_FONT_ENC
    private:

    CEncodings()  {}
    ~CEncodings() {}
#endif

    private:

#ifdef HAVE_FONT_ENC
    QStringList  itsList,
                 itsExtraList;
#else
    static T8Bit their8Bit[];
#endif
};

}

#endif
